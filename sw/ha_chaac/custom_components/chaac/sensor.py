"""Sensors for Chaac weather stations (one device per station UID)."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass
from datetime import datetime

from homeassistant.components.sensor import (
    RestoreSensor,
    SensorDeviceClass,
    SensorEntity,
    SensorEntityDescription,
    SensorStateClass,
)
from homeassistant.const import (
    DEGREE,
    PERCENTAGE,
    SIGNAL_STRENGTH_DECIBELS,
    SIGNAL_STRENGTH_DECIBELS_MILLIWATT,
    EntityCategory,
    UnitOfElectricPotential,
    UnitOfPrecipitationDepth,
    UnitOfPressure,
    UnitOfSpeed,
    UnitOfTemperature,
)
from homeassistant.core import HomeAssistant, callback
from homeassistant.helpers import entity_registry as er
from homeassistant.helpers.device_registry import DeviceInfo
from homeassistant.helpers.dispatcher import async_dispatcher_connect
from homeassistant.helpers.entity_platform import AddConfigEntryEntitiesCallback

from . import ChaacConfigEntry
from .const import DOMAIN, signal_new_station, signal_reading, signal_tick, uid_str
from .protocol import PACKET_TYPE_WEATHER_V1P1, WeatherReading
from .receiver import ChaacReceiver


@dataclass(frozen=True, kw_only=True)
class ChaacSensorDescription(SensorEntityDescription):
    value_fn: Callable[[WeatherReading], float | int | None]
    # Only created for stations sending these packet types (None = all)
    packet_types: frozenset[int] | None = None


V1P1_ONLY = frozenset({PACKET_TYPE_WEATHER_V1P1})

SENSORS: tuple[ChaacSensorDescription, ...] = (
    ChaacSensorDescription(
        key="temperature",
        device_class=SensorDeviceClass.TEMPERATURE,
        state_class=SensorStateClass.MEASUREMENT,
        native_unit_of_measurement=UnitOfTemperature.CELSIUS,
        suggested_display_precision=1,
        value_fn=lambda r: r.values["temperature"],
    ),
    ChaacSensorDescription(
        key="alt_temperature",
        translation_key="alt_temperature",
        device_class=SensorDeviceClass.TEMPERATURE,
        state_class=SensorStateClass.MEASUREMENT,
        native_unit_of_measurement=UnitOfTemperature.CELSIUS,
        suggested_display_precision=1,
        packet_types=V1P1_ONLY,
        value_fn=lambda r: r.values["alt_temperature"],
    ),
    ChaacSensorDescription(
        key="humidity",
        device_class=SensorDeviceClass.HUMIDITY,
        state_class=SensorStateClass.MEASUREMENT,
        native_unit_of_measurement=PERCENTAGE,
        suggested_display_precision=1,
        value_fn=lambda r: r.values["humidity"],
    ),
    ChaacSensorDescription(
        key="pressure",
        device_class=SensorDeviceClass.ATMOSPHERIC_PRESSURE,
        state_class=SensorStateClass.MEASUREMENT,
        native_unit_of_measurement=UnitOfPressure.HPA,
        suggested_display_precision=1,
        value_fn=lambda r: r.values["pressure"],
    ),
    ChaacSensorDescription(
        key="wind_speed",
        device_class=SensorDeviceClass.WIND_SPEED,
        state_class=SensorStateClass.MEASUREMENT,
        native_unit_of_measurement=UnitOfSpeed.KILOMETERS_PER_HOUR,
        suggested_display_precision=1,
        value_fn=lambda r: r.values["wind_speed"],
    ),
    ChaacSensorDescription(
        key="gust_speed",
        translation_key="gust_speed",
        device_class=SensorDeviceClass.WIND_SPEED,
        state_class=SensorStateClass.MEASUREMENT,
        native_unit_of_measurement=UnitOfSpeed.KILOMETERS_PER_HOUR,
        suggested_display_precision=1,
        packet_types=V1P1_ONLY,
        value_fn=lambda r: r.values["gust_speed"],
    ),
    ChaacSensorDescription(
        key="wind_direction",
        device_class=SensorDeviceClass.WIND_DIRECTION,
        state_class=SensorStateClass.MEASUREMENT_ANGLE,
        native_unit_of_measurement=DEGREE,
        suggested_display_precision=0,
        value_fn=lambda r: r.values["wind_direction"],
    ),
    ChaacSensorDescription(
        key="battery",
        translation_key="battery",
        device_class=SensorDeviceClass.VOLTAGE,
        state_class=SensorStateClass.MEASUREMENT,
        native_unit_of_measurement=UnitOfElectricPotential.VOLT,
        suggested_display_precision=2,
        entity_category=EntityCategory.DIAGNOSTIC,
        value_fn=lambda r: r.values["battery"],
    ),
    ChaacSensorDescription(
        key="solar_panel",
        translation_key="solar_panel",
        device_class=SensorDeviceClass.VOLTAGE,
        state_class=SensorStateClass.MEASUREMENT,
        native_unit_of_measurement=UnitOfElectricPotential.VOLT,
        suggested_display_precision=2,
        entity_category=EntityCategory.DIAGNOSTIC,
        value_fn=lambda r: r.values["solar_panel"],
    ),
    ChaacSensorDescription(
        key="rssi",
        device_class=SensorDeviceClass.SIGNAL_STRENGTH,
        state_class=SensorStateClass.MEASUREMENT,
        native_unit_of_measurement=SIGNAL_STRENGTH_DECIBELS_MILLIWATT,
        entity_category=EntityCategory.DIAGNOSTIC,
        value_fn=lambda r: r.rssi,
    ),
    ChaacSensorDescription(
        key="snr",
        translation_key="snr",
        state_class=SensorStateClass.MEASUREMENT,
        native_unit_of_measurement=SIGNAL_STRENGTH_DECIBELS,
        entity_category=EntityCategory.DIAGNOSTIC,
        value_fn=lambda r: r.snr,
    ),
)

RAIN = SensorEntityDescription(
    key="rain",
    translation_key="rain",
    device_class=SensorDeviceClass.PRECIPITATION,
    state_class=SensorStateClass.TOTAL_INCREASING,
    native_unit_of_measurement=UnitOfPrecipitationDepth.MILLIMETERS,
    suggested_display_precision=1,
)

LAST_PACKET = SensorEntityDescription(
    key="last_packet",
    translation_key="last_packet",
    device_class=SensorDeviceClass.TIMESTAMP,
    entity_category=EntityCategory.DIAGNOSTIC,
)

DESCRIPTIONS: dict[str, SensorEntityDescription] = {
    d.key: d for d in (*SENSORS, RAIN, LAST_PACKET)
}


def _entities_for(
    receiver: ChaacReceiver, uid: int, keys: set[str] | None, packet_type: int | None
) -> list[SensorEntity]:
    """Entities for a station: either the keys already registered, or by packet type."""

    def wanted(key: str, packet_types: frozenset[int] | None) -> bool:
        if keys is not None:
            return key in keys
        return packet_types is None or packet_type in packet_types

    entities: list[SensorEntity] = [
        ChaacSensor(receiver, uid, desc)
        for desc in SENSORS
        if wanted(desc.key, desc.packet_types)
    ]
    if wanted(RAIN.key, None):
        entities.append(ChaacRainSensor(receiver, uid))
    if wanted(LAST_PACKET.key, None):
        entities.append(ChaacLastPacketSensor(receiver, uid))
    return entities


async def async_setup_entry(
    hass: HomeAssistant,
    entry: ChaacConfigEntry,
    async_add_entities: AddConfigEntryEntitiesCallback,
) -> None:
    receiver = entry.runtime_data

    # Recreate stations we already know so they show up (unavailable) before
    # their first packet after a restart
    known: dict[int, set[str]] = {}
    for reg_entry in er.async_entries_for_config_entry(er.async_get(hass), entry.entry_id):
        uid, _, key = reg_entry.unique_id.partition("_")
        known.setdefault(int(uid, 16), set()).add(key)
    for uid, keys in known.items():
        receiver.stations.add(uid)
        async_add_entities(_entities_for(receiver, uid, keys, None))

    # Stations imported from a chaac.db get their entities before any packet
    for uid_hex, info in receiver.store.data["stations"].items():
        uid = int(uid_hex, 16)
        if uid not in known:
            receiver.stations.add(uid)
            async_add_entities(_entities_for(receiver, uid, None, info["packet_type"]))

    @callback
    def _new_station(uid: int) -> None:
        reading = receiver.latest[uid]
        async_add_entities(_entities_for(receiver, uid, None, reading.packet_type))

    entry.async_on_unload(
        async_dispatcher_connect(hass, signal_new_station(entry.entry_id), _new_station)
    )


class ChaacEntity(SensorEntity):
    _attr_has_entity_name = True
    _attr_should_poll = False

    def __init__(
        self, receiver: ChaacReceiver, uid: int, description: SensorEntityDescription
    ) -> None:
        self.receiver = receiver
        self.uid = uid
        self.entity_description = description
        self._attr_unique_id = f"{uid_str(uid)}_{description.key}"
        self._attr_device_info = DeviceInfo(
            identifiers={(DOMAIN, uid_str(uid))},
            name=receiver.station_info(uid).get("name") or f"Chaac {uid_str(uid)}",
            manufacturer="Chaac",
            model="Weather station",
            serial_number=uid_str(uid),
        )
        self._was_available = False

    @property
    def available(self) -> bool:
        return self.receiver.is_available(self.uid)

    async def async_added_to_hass(self) -> None:
        await super().async_added_to_hass()
        entry_id = self.receiver.entry.entry_id
        self.async_on_remove(
            async_dispatcher_connect(self.hass, signal_reading(entry_id), self._on_reading)
        )
        self.async_on_remove(
            async_dispatcher_connect(self.hass, signal_tick(entry_id), self._on_tick)
        )
        if (reading := self.receiver.latest.get(self.uid)) is not None:
            self._update(reading, first=True)
        self._was_available = self.available

    @callback
    def _on_reading(self, reading: WeatherReading) -> None:
        if reading.uid != self.uid:
            return
        self._update(reading, first=False)
        self._was_available = True
        self.async_write_ha_state()

    @callback
    def _on_tick(self) -> None:
        if self.available != self._was_available:
            self._was_available = self.available
            self.async_write_ha_state()

    @callback
    def _update(self, reading: WeatherReading, first: bool) -> None:
        raise NotImplementedError


class ChaacSensor(ChaacEntity):
    entity_description: ChaacSensorDescription

    @callback
    def _update(self, reading: WeatherReading, first: bool) -> None:
        self._attr_native_value = self.entity_description.value_fn(reading)


class ChaacLastPacketSensor(ChaacEntity):
    def __init__(self, receiver: ChaacReceiver, uid: int) -> None:
        super().__init__(receiver, uid, LAST_PACKET)

    @callback
    def _update(self, reading: WeatherReading, first: bool) -> None:
        self._attr_native_value = self.receiver.last_seen.get(self.uid)


class ChaacRainSensor(ChaacEntity, RestoreSensor):
    """Running rain total. Stations send rain since their previous packet."""

    def __init__(self, receiver: ChaacReceiver, uid: int) -> None:
        super().__init__(receiver, uid, RAIN)
        self._attr_native_value = 0.0

    async def async_added_to_hass(self) -> None:
        last = await self.async_get_last_sensor_data()
        if last is not None and isinstance(last.native_value, (int, float)):
            self._attr_native_value = float(last.native_value)
        else:
            # New meter: continue from the imported history, if any. HA has
            # recorded nothing for it yet, so this is its starting point.
            self._attr_native_value = float(
                self.receiver.station_info(self.uid).get("rain_start", 0.0)
            )
        await super().async_added_to_hass()

    @callback
    def _update(self, reading: WeatherReading, first: bool) -> None:
        # The reading that created the device was already handled before this
        # entity existed; counting it after a restore could double count.
        if first:
            return
        self._attr_native_value = round(self._attr_native_value + reading.rain_mm, 4)
