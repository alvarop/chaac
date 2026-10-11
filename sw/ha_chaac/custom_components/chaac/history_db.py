"""Read history from an old chaac.db (sw/chaac/chaac/chaacdb.py).

Pure Python (no Home Assistant imports) so it can be tested on its own.

hour_samples holds hourly averages stamped at mid-hour (rain is the hour's
total, wind_dir a speed-weighted vector mean). minute_samples only keeps a few
days and is not used. wind_gust is skipped: the old logger always stored 0.
"""

from __future__ import annotations

from dataclasses import dataclass, field
from datetime import UTC, datetime
import math
import sqlite3

from .protocol import PACKET_TYPE_WEATHER_V1P0, PACKET_TYPE_WEATHER_V1P1

# hour_samples column -> sensor key (unique_id is "<UID>_<key>")
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

HOUR = 3600


class InvalidHistoryDb(Exception):
    """Not a chaac.db, or nothing to import."""


@dataclass
class StationHistory:
    uid: int
    name: str | None
    packet_type: int
    # sensor key -> [(hour start, value)], sorted by time
    series: dict[str, list[tuple[datetime, float]]] = field(default_factory=dict)

    @property
    def first_hour(self) -> datetime:
        return min(rows[0][0] for rows in self.series.values() if rows)

    @property
    def end(self) -> datetime:
        """Start of the first hour not covered by the history."""
        last = max(rows[-1][0] for rows in self.series.values() if rows)
        return datetime.fromtimestamp(last.timestamp() + HOUR, UTC)

    @property
    def rain_total(self) -> float:
        return round(sum(v for _, v in self.series.get("rain", [])), 4)

    @property
    def hours(self) -> int:
        return max(len(rows) for rows in self.series.values())


def read_history(path: str) -> list[StationHistory]:
    try:
        db = sqlite3.connect(f"file:{path}?mode=ro", uri=True)
        db.execute("SELECT timestamp, uid FROM hour_samples LIMIT 1")
    except sqlite3.DatabaseError as err:
        raise InvalidHistoryDb(str(err)) from err

    try:
        names = dict(db.execute("SELECT uid, name FROM devices"))
    except sqlite3.DatabaseError:
        names = {}

    stations = []
    for (uid,) in db.execute("SELECT DISTINCT uid FROM hour_samples ORDER BY uid"):
        uid = int(uid)
        station = StationHistory(uid, names.get(uid) or None, PACKET_TYPE_WEATHER_V1P0)
        for column, key in COLUMNS.items():
            station.series[key] = [
                (datetime.fromtimestamp(ts - HOUR // 2, UTC), float(value))
                for ts, value in db.execute(
                    f"SELECT timestamp, {column} FROM hour_samples "
                    "WHERE uid = ? ORDER BY timestamp",
                    (uid,),
                )
                if value is not None and not math.isnan(value)
            ]
        # Only V1.1 stations have the second temperature sensor
        if any(v != 0 for _, v in station.series["alt_temperature"]):
            station.packet_type = PACKET_TYPE_WEATHER_V1P1
        else:
            del station.series["alt_temperature"]
        if any(station.series.values()):
            stations.append(station)
    db.close()

    if not stations:
        raise InvalidHistoryDb("no hourly samples")
    return stations


def build_rows(
    rows: list[tuple[datetime, float]], *, has_sum: bool, circular: bool
) -> list[dict]:
    """Statistics rows for HA's recorder (StatisticData)."""
    if has_sum:
        out = []
        total = 0.0
        for start, value in rows:
            total += value
            out.append({"start": start, "state": round(total, 4), "sum": round(total, 4)})
        return out
    if circular:
        # HA weights hours by the time covered (seconds) when it rolls hourly
        # circular means up into days and months; each hour here is complete
        return [{"start": s, "mean": v, "mean_weight": float(HOUR)} for s, v in rows]
    return [{"start": s, "mean": v, "min": v, "max": v} for s, v in rows]
