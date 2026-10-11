import importlib.util
import pathlib
import random
import struct
import sys

import pytest

# Load protocol.py directly so the tests don't need Home Assistant installed
_PATH = pathlib.Path(__file__).parents[1] / "custom_components" / "chaac" / "protocol.py"
_spec = importlib.util.spec_from_file_location("chaac_protocol", _PATH)
protocol = importlib.util.module_from_spec(_spec)
sys.modules[_spec.name] = protocol
_spec.loader.exec_module(protocol)

# Frame captured from the Sunnyvale "Garden" station on 2026-10-11
GARDEN_FRAME = bytes.fromhex(
    "55aa2600126335140b63270000bc0c00b4006402e0064407d8139cff0710c619"
    "000000000000cdff0d00b864"
)


def test_crc_matches_serial_packet_library():
    serial_packet = pytest.importorskip("serial_packet.crc")
    rng = random.Random(1)
    for n in (0, 1, 5, 38, 300):
        data = bytes(rng.randrange(256) for _ in range(n))
        assert protocol.crc16(data) == serial_packet.crc16(data)


def test_encode_matches_captured_frame():
    payload = GARDEN_FRAME[4:-2]
    assert protocol.encode_frame(payload) == GARDEN_FRAME


def test_decode_garden_frame():
    (payload,) = protocol.FrameDecoder().feed(GARDEN_FRAME)
    reading = protocol.decode_payload(payload)
    assert reading.uid == 0x14356312
    assert reading.packet_type == protocol.PACKET_TYPE_WEATHER_V1P1
    assert reading.sample == 10083
    assert reading.values == {
        "temperature": 17.6,
        "alt_temperature": 18.6,
        "humidity": 50.8,
        "pressure": 999.0,
        "battery": 4.103,
        "solar_panel": 6.598,
        "wind_speed": 1.8,
        "gust_speed": 6.12,
        "wind_direction": 326.0,
    }
    assert reading.rain_mm == 0
    assert (reading.rssi, reading.snr) == (-51, 13)


def test_decode_matches_chaac_library():
    packets = pytest.importorskip("chaac.packets")
    payload = GARDEN_FRAME[4:-2]
    ref = packets.WeatherPacketV1P1.decode(payload)
    reading = protocol.decode_payload(payload)
    assert reading.values["temperature"] == ref.temperature / 100
    assert reading.values["wind_direction"] == ref.wind_dir_deg / 10
    assert reading.values["gust_speed"] == ref.gust_speed / 100


def test_decoder_resyncs_on_garbage_and_split_reads():
    rng = random.Random(2)
    junk = bytes(rng.randrange(256) for _ in range(100)).replace(b"\x55\xaa", b"")
    # false start with a huge length, then a corrupted copy, then a good frame
    corrupted = GARDEN_FRAME[:10] + b"\x00" + GARDEN_FRAME[11:]
    stream = junk + b"\x55\xaa\xff\xff" + corrupted + GARDEN_FRAME + GARDEN_FRAME
    dec = protocol.FrameDecoder()
    out = []
    for i in range(0, len(stream), 7):
        out += dec.feed(stream[i : i + 7])
    assert out == [GARDEN_FRAME[4:-2]] * 2
    assert dec.crc_errors >= 1


def test_invalid_sensor_values_are_none():
    payload = bytearray(GARDEN_FRAME[4:-2])
    # temperature, alt_temperature, humidity, pressure, battery offsets (V1.1)
    struct.pack_into("<hhHhH", payload, 16, -27300, -27300, 0, -32768, 0)
    values = protocol.decode_payload(bytes(payload)).values
    for key in ("temperature", "alt_temperature", "humidity", "pressure", "battery"):
        assert values[key] is None, key
    # chaac_wxt "no Vaisala data" packet: wind is stale, so drop it too
    for key in ("wind_speed", "gust_speed", "wind_direction"):
        assert values[key] is None, key


def test_single_invalid_sensor_keeps_wind():
    payload = bytearray(GARDEN_FRAME[4:-2])
    struct.pack_into("<h", payload, 22, -32768)  # pressure only
    values = protocol.decode_payload(bytes(payload)).values
    assert values["pressure"] is None
    assert values["wind_speed"] == 1.8


def test_ping_pong():
    pong = struct.pack("<IBHH", 0, protocol.PACKET_TYPE_USB_CMD, protocol.USB_CMD_PONG, 0)
    (payload,) = protocol.FrameDecoder().feed(protocol.encode_frame(pong))
    assert protocol.is_pong(payload)
    assert not protocol.is_pong(GARDEN_FRAME[4:-2])
