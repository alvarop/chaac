"""Load the integration's pure modules without Home Assistant installed."""

import importlib
import pathlib
import sys
import types

_PKG_DIR = pathlib.Path(__file__).parents[1] / "custom_components" / "chaac"

# A bare package for the integration directory, so its __init__.py (which
# needs Home Assistant) is not run but relative imports still work
_pkg = types.ModuleType("chaac_integration")
_pkg.__path__ = [str(_PKG_DIR)]
sys.modules.setdefault("chaac_integration", _pkg)


def load(name: str):
    return importlib.import_module(f"chaac_integration.{name}")
