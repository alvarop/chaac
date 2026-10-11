import importlib.util
import pathlib
import sys

import pytest

pytest.importorskip("websockets")

_PATH = pathlib.Path(__file__).parents[1] / "scripts" / "import_chaac_db.py"
_spec = importlib.util.spec_from_file_location("import_chaac_db", _PATH)
importer = importlib.util.module_from_spec(_spec)
sys.modules[_spec.name] = importer
_spec.loader.exec_module(importer)

HOUR = 3600
ROWS = [(1_700_000_000 + i * HOUR, v) for i, v in enumerate([0.0, 1.5, 0.0, 2.25])]


def test_rain_running_total_from_zero():
    meta = {"has_sum": True, "mean_type": 0}
    stats = importer.build_stats("rain", meta, ROWS)
    # HA takes the first row's change against 0, so the first sum must be the
    # first hour's own amount, never an offset
    assert stats[0]["sum"] == ROWS[0][1]
    assert [s["sum"] for s in stats] == [0.0, 1.5, 1.5, 3.75]
    assert all(s["state"] == s["sum"] for s in stats)
    # hourly change between consecutive sums is the original hourly amount
    diffs = [b["sum"] - a["sum"] for a, b in zip(stats, stats[1:])]
    assert diffs == [v for _, v in ROWS[1:]]


def test_circular_mean_has_no_min_max():
    meta = {"has_sum": False, "mean_type": importer.MEAN_TYPE_CIRCULAR}
    stats = importer.build_stats("wind_direction", meta, ROWS)
    assert set(stats[0]) == {"start", "mean"}


def test_arithmetic_mean_min_max_equal():
    meta = {"has_sum": False, "mean_type": 1}
    stats = importer.build_stats("temperature", meta, ROWS)
    assert all(s["mean"] == s["min"] == s["max"] for s in stats)
    assert stats[0]["start"] == "2023-11-14T22:13:20+00:00"
