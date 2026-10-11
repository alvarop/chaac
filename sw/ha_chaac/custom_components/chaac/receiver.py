"""Serial connection to the Chaac USB LoRa receiver."""

from __future__ import annotations

import asyncio
from datetime import datetime, timedelta
import logging

import serial
import serial_asyncio_fast

from homeassistant.config_entries import ConfigEntry
from homeassistant.core import HomeAssistant, callback
from homeassistant.helpers.dispatcher import async_dispatcher_send
from homeassistant.helpers.event import async_track_time_interval
from homeassistant.util import dt as dt_util

from .const import (
    BAUD_RATE,
    CONF_APPROVED,
    CONF_AUTO_ADD,
    CONF_SERIAL_PORT,
    CONF_UNAVAILABLE_AFTER,
    DEFAULT_AUTO_ADD,
    DEFAULT_UNAVAILABLE_AFTER,
    EVENT_RESET,
    signal_new_station,
    signal_reading,
    signal_tick,
    uid_str,
)
from .protocol import FrameDecoder, MemfaultChunk, ResetEvent, WeatherReading, decode_payload

_LOGGER = logging.getLogger(__name__)

RECONNECT_MAX_DELAY = 60  # seconds
TICK_INTERVAL = timedelta(seconds=30)


class ChaacReceiver:
    """Reads the receiver and fans readings out to entities.

    Stations are identified by UID. A station gets entities the first time it
    is heard, if auto-add is on or it was approved in the options.
    """

    def __init__(self, hass: HomeAssistant, entry: ConfigEntry) -> None:
        self.hass = hass
        self.entry = entry
        self.port: str = entry.data[CONF_SERIAL_PORT]
        self.auto_add: bool = entry.options.get(CONF_AUTO_ADD, DEFAULT_AUTO_ADD)
        self.approved: set[int] = {
            int(uid, 16) for uid in entry.options.get(CONF_APPROVED, [])
        }
        self.unavailable_after = timedelta(
            minutes=entry.options.get(CONF_UNAVAILABLE_AFTER, DEFAULT_UNAVAILABLE_AFTER)
        )
        self.connected = False
        # UIDs that already have entities (filled by the sensor platform)
        self.stations: set[int] = set()
        # UIDs heard but not allowed to create devices
        self.pending: set[int] = set()
        self.latest: dict[int, WeatherReading] = {}
        self.last_seen: dict[int, datetime] = {}
        self._last_sample: dict[int, int] = {}
        self._decoder = FrameDecoder()

    def start(self) -> None:
        self.entry.async_create_background_task(
            self.hass, self._run(), f"chaac receiver {self.port}"
        )
        self.entry.async_on_unload(
            async_track_time_interval(self.hass, self._tick, TICK_INTERVAL)
        )

    def is_available(self, uid: int) -> bool:
        seen = self.last_seen.get(uid)
        return seen is not None and dt_util.utcnow() - seen < self.unavailable_after

    @callback
    def _tick(self, _now: datetime) -> None:
        async_dispatcher_send(self.hass, signal_tick(self.entry.entry_id))

    async def _run(self) -> None:
        delay = 1
        while True:
            try:
                reader, writer = await serial_asyncio_fast.open_serial_connection(
                    url=self.port, baudrate=BAUD_RATE
                )
            except (OSError, serial.SerialException) as err:
                if delay == 1:
                    _LOGGER.warning("Cannot open %s: %s", self.port, err)
                await asyncio.sleep(delay)
                delay = min(delay * 2, RECONNECT_MAX_DELAY)
                continue

            _LOGGER.info("Connected to Chaac receiver on %s", self.port)
            self.connected = True
            delay = 1
            self._decoder = FrameDecoder()
            try:
                while True:
                    data = await reader.read(1024)
                    if not data:
                        raise ConnectionError("serial port closed")
                    for payload in self._decoder.feed(data):
                        self._handle(payload)
            except (OSError, serial.SerialException, ConnectionError) as err:
                _LOGGER.warning("Lost connection to %s: %s", self.port, err)
            finally:
                self.connected = False
                writer.close()
            await asyncio.sleep(delay)

    @callback
    def _handle(self, payload: bytes) -> None:
        packet = decode_payload(payload)

        if isinstance(packet, WeatherReading):
            self._handle_reading(packet)
        elif isinstance(packet, ResetEvent):
            _LOGGER.info("Station %s reset", uid_str(packet.uid))
            self.hass.bus.async_fire(EVENT_RESET, {"uid": uid_str(packet.uid)})
        elif isinstance(packet, MemfaultChunk):
            _LOGGER.debug(
                "Memfault chunk: --device-serial %s --encoding hex %s",
                uid_str(packet.uid),
                packet.data.hex().upper(),
            )

    @callback
    def _handle_reading(self, reading: WeatherReading) -> None:
        uid = reading.uid
        if uid not in self.stations:
            if not (self.auto_add or uid in self.approved):
                if uid not in self.pending:
                    _LOGGER.info(
                        "Ignoring unapproved station %s (approve it in the options)",
                        uid_str(uid),
                    )
                    self.pending.add(uid)
                return
            self.stations.add(uid)
            self._last_sample[uid] = reading.sample
            self.latest[uid] = reading
            self.last_seen[uid] = dt_util.utcnow()
            async_dispatcher_send(self.hass, signal_new_station(self.entry.entry_id), uid)
            return

        last = self._last_sample.get(uid)
        if last is not None and reading.sample != last + 1:
            if reading.sample > last + 1:
                _LOGGER.debug(
                    "Station %s: missed %d packet(s)", uid_str(uid), reading.sample - last - 1
                )
            else:
                _LOGGER.info("Station %s: sample counter restarted", uid_str(uid))
        self._last_sample[uid] = reading.sample

        self.latest[uid] = reading
        self.last_seen[uid] = dt_util.utcnow()
        async_dispatcher_send(self.hass, signal_reading(self.entry.entry_id), reading)
