from datetime import UTC, datetime
import sqlite3

import pytest
from conftest import load

history_db = load("history_db")

SCHEMA = """
CREATE TABLE hour_samples(id INTEGER PRIMARY KEY, timestamp INTEGER, uid INTEGER,
  temperature FLOAT, humidity FLOAT, pressure FLOAT, alt_temperature FLOAT,
  light FLOAT, battery FLOAT, rain FLOAT, wind_speed FLOAT, wind_gust FLOAT,
  wind_dir FLOAT, solar_panel FLOAT);
CREATE TABLE devices(uid INTEGER PRIMARY KEY, name TEXT, gps TEXT);
"""
T0 = 1_700_000_000 - 1_700_000_000 % 3600 + 1800  # mid-hour, like chaacdb


def make_db(path, rows, devices=()):
    db = sqlite3.connect(path)
    db.executescript(SCHEMA)
    db.executemany(
        "INSERT INTO hour_samples(timestamp, uid, temperature, humidity, pressure,"
        " alt_temperature, light, battery, rain, wind_speed, wind_gust, wind_dir,"
        " solar_panel) VALUES (?,?,?,?,?,?,0,?,?,?,0,?,?)",
        rows,
    )
    db.executemany("INSERT INTO devices VALUES (?,?,'')", devices)
    db.commit()
    db.close()


def row(i, uid=0x14356312, rain=0.0, alt=18.0):
    return (T0 + i * 3600, uid, 17.0 + i, 50.0, 1000.0, alt, 4.1, rain, 2.0, 90.0, 6.6)


def test_read_history(tmp_path):
    path = tmp_path / "chaac.db"
    make_db(path, [row(0, rain=0.2794), row(1), row(2, rain=0.5588)], [(0x14356312, "Garden")])
    (station,) = history_db.read_history(str(path))

    assert station.name == "Garden"
    assert station.packet_type == 11
    # mid-hour timestamps become hour starts
    assert station.first_hour == datetime.fromtimestamp(T0 - 1800, UTC)
    assert station.end == datetime.fromtimestamp(T0 - 1800 + 3 * 3600, UTC)
    assert station.rain_total == 0.8382
    assert station.hours == 3
    assert "wind_gust" not in station.series and "light" not in station.series
    assert [v for _, v in station.series["wind_direction"]] == [90.0] * 3


def test_v1p0_station_has_no_alt_temperature(tmp_path):
    path = tmp_path / "chaac.db"
    make_db(path, [row(0, uid=1, alt=0.0), row(1, uid=1, alt=0.0)])
    (station,) = history_db.read_history(str(path))
    assert station.packet_type == 10
    assert station.name is None
    assert "alt_temperature" not in station.series


@pytest.mark.parametrize("content", [b"", b"not a database"])
def test_invalid_db(tmp_path, content):
    path = tmp_path / "chaac.db"
    path.write_bytes(content)
    with pytest.raises(history_db.InvalidHistoryDb):
        history_db.read_history(str(path))


ROWS = [(datetime.fromtimestamp(T0 - 1800 + i * 3600, UTC), v)
        for i, v in enumerate([0.0, 1.5, 0.0, 2.25])]


def test_rain_rows_running_total_from_zero():
    out = history_db.build_rows(ROWS, has_sum=True, circular=False)
    # HA takes the first row's change against 0
    assert out[0]["sum"] == ROWS[0][1]
    assert [r["sum"] for r in out] == [0.0, 1.5, 1.5, 3.75]
    assert all(r["state"] == r["sum"] for r in out)


def test_circular_rows_are_weighted_by_full_hour():
    out = history_db.build_rows(ROWS, has_sum=False, circular=True)
    assert all(set(r) == {"start", "mean", "mean_weight"} for r in out)
    assert all(r["mean_weight"] == 3600 for r in out)


def test_mean_rows():
    out = history_db.build_rows(ROWS, has_sum=False, circular=False)
    assert all(r["mean"] == r["min"] == r["max"] for r in out)
