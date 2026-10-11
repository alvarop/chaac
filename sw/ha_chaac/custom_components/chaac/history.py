"""Import chaac.db history into Home Assistant long-term statistics.

Runs once, when the receiver is added with a chaac.db, before the receiver
starts listening:

1. Stations from the DB get their devices up front, with their old names, and
   each rain meter starts at the station's imported rain total. Because HA has
   not recorded anything for the meter yet, the recorder takes that first
   reading as its starting point instead of counting it as rain.
2. The hourly history goes into each sensor's own statistics, so old and new
   data are one series.
3. The recorder continues the rain sum from its own 5-minute statistics, so
   the live sum starts at 0. Once the first live 5-minute row exists, the live
   rain statistics are shifted up by the imported total (the same operation as
   "Adjust sum" in Developer tools > Statistics). Pending shifts are kept in
   storage and resumed after a restart.
"""

from __future__ import annotations

from datetime import datetime, timedelta
import logging
from pathlib import Path

from homeassistant.components.recorder import get_instance
from homeassistant.components.recorder.models import StatisticMeanType, StatisticMetaData
from homeassistant.components.recorder.statistics import (
    STATISTIC_UNIT_TO_UNIT_CONVERTER,
    async_import_statistics,
    statistics_during_period,
)
from homeassistant.components.sensor import (
    DOMAIN as SENSOR_DOMAIN,
    SensorEntityDescription,
    SensorStateClass,
)
from homeassistant.components.sensor.const import UNIT_CONVERTERS
from homeassistant.core import HomeAssistant, callback
from homeassistant.helpers import entity_registry as er
from homeassistant.helpers.event import async_track_time_interval
from homeassistant.helpers.storage import Store
from homeassistant.util import dt as dt_util

from .const import DOMAIN, uid_str
from .history_db import StationHistory, build_rows

_LOGGER = logging.getLogger(__name__)

STORAGE_VERSION = 1
SHIFT_CHECK_INTERVAL = timedelta(minutes=1)


class ChaacStore:
    """Per-receiver storage: imported stations and pending rain shifts.

    {"stations": {"<UID>": {"name", "packet_type", "rain_start"}},
     "pending_shifts": {"<UID>": {"start": iso, "total": float}}}
    """

    def __init__(self, hass: HomeAssistant, entry_id: str) -> None:
        self._store: Store[dict] = Store(hass, STORAGE_VERSION, f"{DOMAIN}.{entry_id}")
        self.data: dict = {"stations": {}, "pending_shifts": {}}

    async def async_load(self) -> None:
        if (data := await self._store.async_load()) is not None:
            self.data = data

    async def async_save(self) -> None:
        await self._store.async_save(self.data)

    async def async_remove(self) -> None:
        await self._store.async_remove()


def register_stations(store: ChaacStore, stations: list[StationHistory]) -> None:
    for station in stations:
        store.data["stations"][uid_str(station.uid)] = {
            "name": station.name,
            "packet_type": station.packet_type,
            "rain_start": station.rain_total,
        }


def _unit_class(description: SensorEntityDescription) -> str | None:
    """Same rule the sensor recorder uses, so the metadata matches."""
    unit = description.native_unit_of_measurement
    if (
        description.device_class
        and (conv := UNIT_CONVERTERS.get(description.device_class))
        and unit in conv.VALID_UNITS
    ):
        return conv.UNIT_CLASS
    if conv := STATISTIC_UNIT_TO_UNIT_CONVERTER.get(unit):
        return conv.UNIT_CLASS
    return None


@callback
def async_import_history(
    hass: HomeAssistant,
    store: ChaacStore,
    stations: list[StationHistory],
    descriptions: dict[str, SensorEntityDescription],
) -> None:
    """Queue the statistics import. Entities must already be registered."""
    registry = er.async_get(hass)
    for station in stations:
        uid = uid_str(station.uid)
        for key, rows in station.series.items():
            if not rows:
                continue
            description = descriptions[key]
            entity_id = registry.async_get_entity_id(SENSOR_DOMAIN, DOMAIN, f"{uid}_{key}")
            if entity_id is None:
                _LOGGER.error("No entity for %s %s, not importing it", uid, key)
                continue
            has_sum = description.state_class == SensorStateClass.TOTAL_INCREASING
            circular = description.state_class == SensorStateClass.MEASUREMENT_ANGLE
            metadata: StatisticMetaData = {
                "statistic_id": entity_id,
                "source": "recorder",
                "name": None,
                "unit_of_measurement": description.native_unit_of_measurement,
                "unit_class": _unit_class(description),
                "has_sum": has_sum,
                "mean_type": (
                    StatisticMeanType.NONE
                    if has_sum
                    else StatisticMeanType.CIRCULAR
                    if circular
                    else StatisticMeanType.ARITHMETIC
                ),
            }
            async_import_statistics(
                hass, metadata, build_rows(rows, has_sum=has_sum, circular=circular)
            )

        if station.rain_total > 0:
            store.data["pending_shifts"][uid] = {
                "start": station.end.isoformat(),
                "total": station.rain_total,
            }
        _LOGGER.info(
            "Imported %d hours for station %s (%s), %s to %s, rain %.1f mm",
            station.hours,
            uid,
            station.name,
            station.first_hour.isoformat(),
            station.end.isoformat(),
            station.rain_total,
        )


async def _async_shift_rain(hass: HomeAssistant, store: ChaacStore, uid: str) -> bool:
    """Shift the live rain sum once its first 5-minute statistics exist."""
    shift = store.data["pending_shifts"][uid]
    entity_id = er.async_get(hass).async_get_entity_id(SENSOR_DOMAIN, DOMAIN, f"{uid}_rain")
    if entity_id is None:
        return False
    start = dt_util.parse_datetime(shift["start"])
    live = await get_instance(hass).async_add_executor_job(
        statistics_during_period,
        hass,
        start,
        None,
        {entity_id},
        "5minute",
        None,
        {"sum"},
    )
    if not live.get(entity_id):
        return False
    get_instance(hass).async_adjust_statistics(entity_id, start, shift["total"], "mm")
    _LOGGER.info(
        "Shifted live rain statistics of %s by %.3f mm from %s",
        entity_id,
        shift["total"],
        start.isoformat(),
    )
    return True


@callback
def async_track_pending_shifts(hass: HomeAssistant, store: ChaacStore):
    """Apply pending rain shifts as soon as possible. Returns an unsubscribe."""
    unsub = None

    async def _check(_now: datetime | None = None) -> None:
        nonlocal unsub
        done = [
            uid
            for uid in list(store.data["pending_shifts"])
            if await _async_shift_rain(hass, store, uid)
        ]
        for uid in done:
            del store.data["pending_shifts"][uid]
        if done:
            await store.async_save()
        if not store.data["pending_shifts"] and unsub is not None:
            unsub()
            unsub = None

    if not store.data["pending_shifts"]:
        return lambda: None
    unsub = async_track_time_interval(hass, _check, SHIFT_CHECK_INTERVAL)

    def _unsubscribe() -> None:
        if unsub is not None:
            unsub()

    return _unsubscribe


def remove_file(path: str) -> None:
    Path(path).unlink(missing_ok=True)
