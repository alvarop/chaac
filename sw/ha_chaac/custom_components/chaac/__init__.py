"""Chaac weather stations via a USB LoRa receiver."""

from __future__ import annotations

import logging

from homeassistant.config_entries import ConfigEntry
from homeassistant.const import Platform
from homeassistant.core import HomeAssistant

from .const import CONF_HISTORY_DB
from .history import (
    ChaacStore,
    async_import_history,
    async_track_pending_shifts,
    register_stations,
    remove_file,
)
from .history_db import InvalidHistoryDb, read_history
from .receiver import ChaacReceiver

_LOGGER = logging.getLogger(__name__)

PLATFORMS = [Platform.SENSOR]

type ChaacConfigEntry = ConfigEntry[ChaacReceiver]


async def async_setup_entry(hass: HomeAssistant, entry: ChaacConfigEntry) -> bool:
    store = ChaacStore(hass, entry.entry_id)
    await store.async_load()
    receiver = ChaacReceiver(hass, entry, store)
    entry.runtime_data = receiver

    history = None
    if (path := entry.data.get(CONF_HISTORY_DB)) and not store.data.get("history_imported"):
        try:
            history = await hass.async_add_executor_job(read_history, path)
        except (InvalidHistoryDb, OSError) as err:
            _LOGGER.error("Cannot import history from %s: %s", path, err)
        else:
            register_stations(store, history)

    # Platforms before the receiver starts, so known and imported stations
    # have their entities before packets arrive
    await hass.config_entries.async_forward_entry_setups(entry, PLATFORMS)

    if history is not None:
        from .sensor import DESCRIPTIONS  # noqa: PLC0415 (sensor imports this module)

        async_import_history(hass, store, history, DESCRIPTIONS)
        store.data["history_imported"] = True
        await store.async_save()
        await hass.async_add_executor_job(remove_file, path)

    entry.async_on_unload(async_track_pending_shifts(hass, store))
    receiver.start()
    entry.async_on_unload(entry.add_update_listener(_async_reload))
    return True


async def async_unload_entry(hass: HomeAssistant, entry: ChaacConfigEntry) -> bool:
    return await hass.config_entries.async_unload_platforms(entry, PLATFORMS)


async def async_remove_entry(hass: HomeAssistant, entry: ChaacConfigEntry) -> None:
    await ChaacStore(hass, entry.entry_id).async_remove()
    if path := entry.data.get(CONF_HISTORY_DB):
        await hass.async_add_executor_job(remove_file, path)


async def _async_reload(hass: HomeAssistant, entry: ChaacConfigEntry) -> None:
    await hass.config_entries.async_reload(entry.entry_id)
