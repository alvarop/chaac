#!/usr/bin/env python3
"""Import hourly history from a chaac.db into Home Assistant long-term statistics.

Rows go into each Chaac sensor's own statistics, so old and new data show as
one graph. Only hours before the sensor's first existing statistics row are
imported; nothing HA recorded itself is overwritten. The one change to live
data is the rain sum shift described below.

    import_chaac_db.py --db chaac.db --url ws://homeassistant.local:8123 \
        --token-file ~/.config/ha_token [--dry-run] [--rename-devices]

Notes
- hour_samples holds hourly averages stamped at mid-hour; HA wants the hour
  start, so timestamps are shifted by -30 min and mean = min = max = value.
- wind_gust is skipped: the old logger always stored 0. light is unused.
- Rain: hourly amounts are summed into a running total starting at 0. The
  live sensor's statistics started their own sum at 0, so afterwards they are
  shifted up by the imported total (recorder/adjust_sum_statistics, the same
  thing as HA's "adjust sum" tool). New rows continue from the shifted 5-minute
  statistics, so the series stays continuous. Run this after the integration
  has been recording for a few minutes, so live 5-minute statistics exist.
"""

from __future__ import annotations

import argparse
import asyncio
import json
import math
import pathlib
import sqlite3
import sys
from datetime import datetime, timezone

import websockets

# hour_samples column -> integration sensor key (unique_id is "<UID>_<key>")
COLUMNS = {
    "temperature": "temperature",
    "alt_temperature": "alt_temperature",
    "humidity": "humidity",
    "pressure": "pressure",
    "wind_speed": "wind_speed",
    "wind_dir": "wind_direction",
    "rain": "rain",
    "battery": "battery",
    "solar_panel": "solar_panel",
}

# Units the chaac.db values are in, checked against HA's statistics metadata
DB_UNITS = {
    "temperature": "°C",
    "alt_temperature": "°C",
    "humidity": "%",
    "pressure": "hPa",
    "wind_speed": "km/h",
    "wind_direction": "°",
    "rain": "mm",
    "battery": "V",
    "solar_panel": "V",
}

MEAN_TYPE_CIRCULAR = 2


class HA:
    def __init__(self, ws):
        self.ws = ws
        self._id = 0

    async def call(self, msg_type: str, **kwargs):
        self._id += 1
        await self.ws.send(json.dumps({"id": self._id, "type": msg_type, **kwargs}))
        while True:
            msg = json.loads(await self.ws.recv())
            if msg.get("id") == self._id:
                break
        if not msg.get("success"):
            raise RuntimeError(f"{msg_type} failed: {msg.get('error')}")
        return msg["result"]


def iso(ts: float) -> str:
    return datetime.fromtimestamp(ts, timezone.utc).isoformat()


def start_epoch(start) -> float:
    # statistics_during_period returns epoch ms (older versions: ISO strings)
    if isinstance(start, (int, float)):
        return start / 1000
    return datetime.fromisoformat(start).timestamp()


async def wait_for_recorder(ha: HA, timeout: float = 600) -> None:
    """Imports and adjustments are queued; wait until the recorder ran them."""
    loop = asyncio.get_running_loop()
    deadline = loop.time() + timeout
    while (await ha.call("recorder/info"))["backlog"]:
        if loop.time() > deadline:
            raise RuntimeError("recorder backlog did not drain")
        await asyncio.sleep(2)


async def adjust_live_sum(ha: HA, entity_id: str, unit: str, cutoff: float, total: float, dry_run: bool) -> bool:
    """Shift live sums (from cutoff on) up by the imported total, once."""
    # A shift from an earlier run may still be queued
    await wait_for_recorder(ha)
    result = await ha.call(
        "recorder/statistics_during_period",
        start_time=iso(cutoff),
        statistic_ids=[entity_id],
        period="5minute",
        types=["sum"],
    )
    rows = result.get(entity_id) or []
    if not rows:
        print(f"  {'':16}    no live 5-minute statistics yet; rerun once the integration has recorded")
        return False
    live_sum = rows[0]["sum"]
    # The live sum starts at 0 (plus any rain in its first 5 minutes); once
    # shifted it is at least the imported total
    if live_sum >= total / 2:
        print(f"  {'':16}    live sum already continues from the import ({live_sum:.3f})")
        return True
    verb = "would shift" if dry_run else "shifted"
    print(f"  {'':16}    {verb} live sum by +{total:.3f} {unit} from {iso(cutoff)[:16]}")
    if not dry_run:
        await ha.call(
            "recorder/adjust_sum_statistics",
            statistic_id=entity_id,
            start_time=iso(cutoff),
            adjustment=total,
            adjustment_unit_of_measurement=unit,
        )
    return True


async def first_stat_hour(ha: HA, statistic_id: str, since: float) -> float | None:
    result = await ha.call(
        "recorder/statistics_during_period",
        start_time=iso(since),
        statistic_ids=[statistic_id],
        period="hour",
        types=["mean", "sum"],
    )
    rows = result.get(statistic_id) or []
    return min((start_epoch(r["start"]) for r in rows), default=None)


def build_stats(key: str, meta: dict, rows: list[tuple[float, float]]) -> list[dict]:
    stats = []
    if meta["has_sum"]:
        cumulative = 0.0
        for start, value in rows:
            cumulative += value
            level = round(cumulative, 4)
            stats.append({"start": iso(start), "state": level, "sum": level})
    elif meta["mean_type"] == MEAN_TYPE_CIRCULAR:
        stats = [{"start": iso(s), "mean": v} for s, v in rows]
    else:
        stats = [{"start": iso(s), "mean": v, "min": v, "max": v} for s, v in rows]
    return stats


async def run(args) -> int:
    db = sqlite3.connect(f"file:{args.db}?mode=ro", uri=True)
    names = dict(db.execute("SELECT uid, name FROM devices"))
    uids = [u for (u,) in db.execute("SELECT DISTINCT uid FROM hour_samples")]
    if args.uid:
        uids = [u for u in uids if f"{u:08X}" in {x.upper() for x in args.uid}]

    token = pathlib.Path(args.token_file).expanduser().read_text().strip()
    async with websockets.connect(f"{args.url}/api/websocket", max_size=None) as ws:
        await ws.recv()
        await ws.send(json.dumps({"type": "auth", "access_token": token}))
        if json.loads(await ws.recv())["type"] != "auth_ok":
            print("Authentication failed", file=sys.stderr)
            return 1
        ha = HA(ws)

        registry = await ha.call("config/entity_registry/list")
        entity_ids = {
            e["unique_id"]: e["entity_id"] for e in registry if e["platform"] == "chaac"
        }
        metadata = {
            m["statistic_id"]: m for m in await ha.call("recorder/list_statistic_ids")
        }

        ok = True
        for uid in uids:
            uid_hex = f"{uid:08X}"
            name = names.get(uid) or uid_hex
            print(f"\n== Station {uid_hex} ({name})")

            if f"{uid_hex}_temperature" not in entity_ids:
                print("  no Chaac entities for this UID in HA, skipping")
                ok = False
                continue

            if args.rename_devices and names.get(uid):
                devices = await ha.call("config/device_registry/list")
                for dev in devices:
                    if ["chaac", uid_hex] in dev["identifiers"]:
                        if not args.dry_run:
                            await ha.call(
                                "config/device_registry/update",
                                device_id=dev["id"],
                                name_by_user=names[uid],
                            )
                        verb = "would rename" if args.dry_run else "renamed"
                        print(f"  device {verb} to {names[uid]!r}")

            for column, key in COLUMNS.items():
                entity_id = entity_ids.get(f"{uid_hex}_{key}")
                meta = metadata.get(entity_id) if entity_id else None
                if meta is None:
                    print(f"  {column:16} no entity/statistics in HA, skipping")
                    continue
                unit = meta["statistics_unit_of_measurement"]
                if unit != DB_UNITS[key]:
                    print(f"  {column:16} unit mismatch: HA {unit!r} vs db {DB_UNITS[key]!r}")
                    ok = False
                    continue

                rows = [
                    (ts - 1800, value)
                    for ts, value in db.execute(
                        f"SELECT timestamp, {column} FROM hour_samples "
                        "WHERE uid = ? ORDER BY timestamp",
                        (uid,),
                    )
                    if value is not None and not math.isnan(value)
                ]
                if not rows:
                    continue

                if args.until:
                    cutoff = datetime.fromisoformat(args.until).timestamp()
                else:
                    cutoff = await first_stat_hour(ha, entity_id, rows[0][0])
                    if cutoff is None:
                        print(f"  {column:16} HA has no statistics yet; use --until")
                        ok = False
                        continue
                rows = [r for r in rows if r[0] < cutoff]

                stats = build_stats(key, meta, rows)
                span = f"{iso(rows[0][0])[:16]} .. {iso(rows[-1][0])[:16]}" if rows else "-"
                print(f"  {column:16} -> {entity_id}: {len(stats)} hours ({span}), "
                      f"live data from {iso(cutoff)[:16]}")
                if not stats:
                    continue

                if not args.dry_run:
                    import_meta = {
                        "statistic_id": entity_id,
                        "source": meta["source"],
                        "name": None,
                        "unit_of_measurement": unit,
                        "unit_class": meta["unit_class"],
                        "has_sum": meta["has_sum"],
                        "mean_type": meta["mean_type"],
                    }
                    for i in range(0, len(stats), args.batch):
                        await ha.call(
                            "recorder/import_statistics",
                            metadata=import_meta,
                            stats=stats[i : i + args.batch],
                        )

                # After the import, so a failed import never leaves live data shifted
                if meta["has_sum"] and stats[-1]["sum"] > 0:
                    ok &= await adjust_live_sum(
                        ha, entity_id, unit, cutoff, stats[-1]["sum"], args.dry_run
                    )

        if not args.dry_run:
            print("\nWaiting for the recorder to write everything...")
            await wait_for_recorder(ha)

    if args.dry_run:
        print("\nDry run, nothing imported.")
    return 0 if ok else 2


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--db", required=True, help="chaac.db (copy)")
    parser.add_argument("--url", required=True, help="e.g. ws://homeassistant.local:8123")
    parser.add_argument("--token-file", required=True, help="long-lived access token")
    parser.add_argument("--uid", action="append", help="only this station UID (hex)")
    parser.add_argument(
        "--until",
        help="ISO time: import every hour before it, replacing statistics already "
        "there (default: stop at each sensor's first existing hour)",
    )
    parser.add_argument("--batch", type=int, default=1000, help="hours per import call")
    parser.add_argument("--rename-devices", action="store_true", help="use names from chaac.db")
    parser.add_argument("--dry-run", action="store_true")
    return asyncio.run(run(parser.parse_args()))


if __name__ == "__main__":
    sys.exit(main())
