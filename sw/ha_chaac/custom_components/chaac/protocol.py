"""Chaac USB LoRa receiver wire protocol.

Pure Python (no Home Assistant imports) so it can be tested on its own.

Serial framing (from the serial-packet library / fw lib/packet):
    0x55 0xAA | len (u16 LE) | payload[len] | crc16 (u16 LE)
CRC-16/CCITT (poly 0x1021, init 0x1D0F) over start bytes, len and payload.

Payload is a chaac packet (packed little-endian structs, see
sw/chaac/chaac/packets.py). Packets the receiver heard over LoRa carry a
4-byte rx-info trailer (rssi i16, snr i8, rsvd u8) as the LAST 4 bytes of the
payload. The receiver places it at the radio packet length, not the chaac
packet length, so there are a few padding bytes between the two.
"""

from __future__ import annotations

import struct
from dataclasses import dataclass, field

START = b"\x55\xaa"
HEADER_LEN = 4
CRC_LEN = 2
# usb_lora BUFFER_SIZE is 300; anything longer is a false start sequence
MAX_PAYLOAD_LEN = 512

PACKET_TYPE_RESET = 1
PACKET_TYPE_MEMFAULT = 2
PACKET_TYPE_WEATHER_V1P0 = 10
PACKET_TYPE_WEATHER_V1P1 = 11
PACKET_TYPE_USB_CMD = 0xE0

USB_CMD_PING = 3
USB_CMD_PONG = 4

RAIN_MM_PER_TIP = 0.2794

_HEADER = struct.Struct("<IB")  # uid, packet_type
_RXINFO = struct.Struct("<hbB")  # rssi, snr, rsvd
_USB_CMD = struct.Struct("<IBHH")  # uid, packet_type, cmd, len
_WEATHER_V1P0 = struct.Struct("<IBLBBHHhHhHH")
_WEATHER_V1P1 = struct.Struct("<IBLHBHHhhHhHH")

# Values the station firmware sends when a sensor read failed
_TEMPERATURE_INVALID = -27300
_PRESSURE_INVALID = -32768


def _make_crc_table() -> list[int]:
    table = []
    for byte in range(256):
        crc = byte << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) if crc & 0x8000 else (crc << 1)
        table.append(crc & 0xFFFF)
    return table


_CRC_TABLE = _make_crc_table()


def crc16(data: bytes, crc: int = 0x1D0F) -> int:
    for byte in data:
        crc = ((crc << 8) & 0xFFFF) ^ _CRC_TABLE[((crc >> 8) ^ byte) & 0xFF]
    return crc


def encode_frame(payload: bytes) -> bytes:
    head = START + struct.pack("<H", len(payload))
    body = head + payload
    return body + struct.pack("<H", crc16(body))


def encode_ping() -> bytes:
    return encode_frame(_USB_CMD.pack(0, PACKET_TYPE_USB_CMD, USB_CMD_PING, 0))


def is_pong(payload: bytes) -> bool:
    if len(payload) < _USB_CMD.size:
        return False
    uid, ptype, cmd, _ = _USB_CMD.unpack_from(payload)
    return uid == 0 and ptype == PACKET_TYPE_USB_CMD and cmd == USB_CMD_PONG


class FrameDecoder:
    """Incremental decoder: feed raw serial bytes, get complete payloads."""

    def __init__(self) -> None:
        self._buf = bytearray()
        self.crc_errors = 0

    def feed(self, data: bytes) -> list[bytes]:
        self._buf += data
        payloads = []
        while True:
            start = self._buf.find(START)
            if start < 0:
                # Keep a trailing 0x55 in case it is the first start byte
                del self._buf[: max(0, len(self._buf) - 1)]
                break
            del self._buf[:start]
            if len(self._buf) < HEADER_LEN:
                break
            (length,) = struct.unpack_from("<H", self._buf, 2)
            if length > MAX_PAYLOAD_LEN:
                del self._buf[:2]
                continue
            total = HEADER_LEN + length + CRC_LEN
            if len(self._buf) < total:
                break
            frame = bytes(self._buf[:total])
            (crc,) = struct.unpack_from("<H", frame, total - CRC_LEN)
            if crc != crc16(frame[:-CRC_LEN]):
                self.crc_errors += 1
                del self._buf[:2]
                continue
            del self._buf[:total]
            payloads.append(frame[HEADER_LEN:-CRC_LEN])
        return payloads


@dataclass
class WeatherReading:
    uid: int
    packet_type: int
    sample: int
    # Converted values in HA units; None means the station flagged a bad read
    values: dict[str, float | None] = field(default_factory=dict)
    # Rain since the previous packet, mm
    rain_mm: float = 0.0
    rssi: int | None = None
    snr: int | None = None


@dataclass
class ResetEvent:
    uid: int


@dataclass
class MemfaultChunk:
    uid: int
    data: bytes


def _rxinfo(payload: bytes, packet_size: int) -> tuple[int | None, int | None]:
    if len(payload) < packet_size + _RXINFO.size:
        return None, None
    rssi, snr, _ = _RXINFO.unpack_from(payload, len(payload) - _RXINFO.size)
    return rssi, snr


def _temperature(raw: int) -> float | None:
    return None if raw == _TEMPERATURE_INVALID else raw / 100.0


def _common_values(temperature, humidity, pressure, battery, solar_panel):
    return {
        "temperature": _temperature(temperature),
        "humidity": humidity / 100.0 if humidity else None,
        "pressure": (
            None if pressure == _PRESSURE_INVALID else (pressure + 100000) / 100.0
        ),
        "battery": battery / 1000.0 if battery else None,
        "solar_panel": solar_panel / 1000.0,
    }


def decode_payload(payload: bytes) -> WeatherReading | ResetEvent | MemfaultChunk | None:
    """Decode one chaac packet. Returns None for types we don't handle."""
    if len(payload) < _HEADER.size:
        return None
    uid, ptype = _HEADER.unpack_from(payload)

    if ptype == PACKET_TYPE_WEATHER_V1P1 and len(payload) >= _WEATHER_V1P1.size:
        (uid, ptype, sample, wind_dir_deg, rain, wind_speed, gust_speed,
         temperature, alt_temperature, humidity, pressure, battery,
         solar_panel) = _WEATHER_V1P1.unpack_from(payload)
        rssi, snr = _rxinfo(payload, _WEATHER_V1P1.size)
        values = _common_values(temperature, humidity, pressure, battery, solar_panel)
        values.update(
            alt_temperature=_temperature(alt_temperature),
            wind_speed=wind_speed / 100.0,
            gust_speed=gust_speed / 100.0,
            wind_direction=wind_dir_deg / 10.0,
        )
        # chaac_wxt: when no Vaisala message arrived in time it marks every WXT
        # field invalid except wind, which still holds the previous reading
        if all(
            values[k] is None
            for k in ("temperature", "alt_temperature", "humidity", "pressure")
        ):
            values.update(wind_speed=None, gust_speed=None, wind_direction=None)
        return WeatherReading(uid, ptype, sample, values, rain * RAIN_MM_PER_TIP, rssi, snr)

    if ptype == PACKET_TYPE_WEATHER_V1P0 and len(payload) >= _WEATHER_V1P0.size:
        (uid, ptype, sample, wind_dir, rain, _rsvd, wind_speed, temperature,
         humidity, pressure, battery, solar_panel) = _WEATHER_V1P0.unpack_from(payload)
        rssi, snr = _rxinfo(payload, _WEATHER_V1P0.size)
        values = _common_values(temperature, humidity, pressure, battery, solar_panel)
        values.update(
            wind_speed=wind_speed / 100.0,
            wind_direction=wind_dir * 22.5,
        )
        return WeatherReading(uid, ptype, sample, values, rain * RAIN_MM_PER_TIP, rssi, snr)

    if ptype == PACKET_TYPE_RESET:
        return ResetEvent(uid)

    if ptype == PACKET_TYPE_MEMFAULT and len(payload) >= 7:
        (length,) = struct.unpack_from("<H", payload, 5)
        return MemfaultChunk(uid, bytes(payload[7 : 7 + length]))

    return None
