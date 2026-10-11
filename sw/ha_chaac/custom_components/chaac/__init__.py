"""Chaac weather stations via a USB LoRa receiver."""

from __future__ import annotations

from homeassistant.config_entries import ConfigEntry
from homeassistant.const import Platform
from homeassistant.core import HomeAssistant

from .receiver import ChaacReceiver

PLATFORMS = [Platform.SENSOR]

type ChaacConfigEntry = ConfigEntry[ChaacReceiver]


async def async_setup_entry(hass: HomeAssistant, entry: ChaacConfigEntry) -> bool:
    receiver = ChaacReceiver(hass, entry)
    entry.runtime_data = receiver
    # Platforms first, so stations known from the registry are registered
    # with the receiver before packets start arriving
    await hass.config_entries.async_forward_entry_setups(entry, PLATFORMS)
    receiver.start()
    entry.async_on_unload(entry.add_update_listener(_async_reload))
    return True


async def async_unload_entry(hass: HomeAssistant, entry: ChaacConfigEntry) -> bool:
    return await hass.config_entries.async_unload_platforms(entry, PLATFORMS)


async def _async_reload(hass: HomeAssistant, entry: ChaacConfigEntry) -> None:
    await hass.config_entries.async_reload(entry.entry_id)
