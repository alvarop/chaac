"""Config flow for Chaac: pick the receiver's serial port."""

from __future__ import annotations

import logging
import os
from pathlib import Path
import shutil
import time
from typing import Any

import serial
import voluptuous as vol

from homeassistant.components.file_upload import process_uploaded_file
from homeassistant.config_entries import ConfigFlow, ConfigFlowResult, OptionsFlow
from homeassistant.core import HomeAssistant, callback
from homeassistant.helpers.selector import (
    BooleanSelector,
    FileSelector,
    FileSelectorConfig,
    NumberSelector,
    NumberSelectorConfig,
    NumberSelectorMode,
    SelectOptionDict,
    SelectSelector,
    SelectSelectorConfig,
    SelectSelectorMode,
)

from .const import (
    BAUD_RATE,
    CONF_APPROVED,
    CONF_AUTO_ADD,
    CONF_HISTORY_DB,
    CONF_SERIAL_PORT,
    CONF_UNAVAILABLE_AFTER,
    DEFAULT_AUTO_ADD,
    DEFAULT_UNAVAILABLE_AFTER,
    DOMAIN,
    uid_str,
)
from .history_db import InvalidHistoryDb, read_history
from .protocol import FrameDecoder, encode_ping, is_pong

_LOGGER = logging.getLogger(__name__)

BY_ID_DIR = "/dev/serial/by-id"
PING_TIMEOUT = 2.0  # seconds


def _list_ports() -> list[SelectOptionDict]:
    """by-id paths survive reboots and replugging; prefer them."""
    try:
        names = sorted(os.listdir(BY_ID_DIR))
    except OSError:
        return []
    # Chaac receivers first
    names.sort(key=lambda n: "chaac" not in n.lower())
    return [SelectOptionDict(value=os.path.join(BY_ID_DIR, n), label=n) for n in names]


def _ping(port: str) -> bool:
    """True if a Chaac receiver answers a ping on this port."""
    decoder = FrameDecoder()
    with serial.serial_for_url(port, baudrate=BAUD_RATE, timeout=0.1) as ser:
        ser.reset_input_buffer()
        ser.write(encode_ping())
        deadline = time.monotonic() + PING_TIMEOUT
        while time.monotonic() < deadline:
            if any(is_pong(p) for p in decoder.feed(ser.read(256))):
                return True
    return False


def _save_history_db(hass: HomeAssistant, file_id: str, dest: Path) -> str:
    """Keep a copy of the uploaded chaac.db for setup, after checking it."""
    with process_uploaded_file(hass, file_id) as uploaded:
        dest.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(uploaded, dest)
    try:
        stations = read_history(str(dest))
    except InvalidHistoryDb:
        dest.unlink(missing_ok=True)
        raise
    return ", ".join(
        f"{s.name or f'{s.uid:08X}'}: {s.hours} hours from {s.first_hour:%Y-%m-%d}"
        for s in stations
    )


class ChaacConfigFlow(ConfigFlow, domain=DOMAIN):
    VERSION = 1

    def __init__(self) -> None:
        self._data: dict[str, Any] = {}

    async def async_step_user(
        self, user_input: dict[str, Any] | None = None
    ) -> ConfigFlowResult:
        errors: dict[str, str] = {}
        if user_input is not None:
            port = user_input[CONF_SERIAL_PORT]
            await self.async_set_unique_id(port)
            self._abort_if_unique_id_configured()
            try:
                ok = await self.hass.async_add_executor_job(_ping, port)
            except (OSError, serial.SerialException):
                errors["base"] = "cannot_connect"
            else:
                if ok:
                    self._data[CONF_SERIAL_PORT] = port
                    return await self.async_step_history()
                errors["base"] = "no_response"

        ports = await self.hass.async_add_executor_job(_list_ports)
        schema = vol.Schema(
            {
                vol.Required(CONF_SERIAL_PORT): SelectSelector(
                    SelectSelectorConfig(
                        options=ports, custom_value=True, mode=SelectSelectorMode.DROPDOWN
                    )
                )
            }
        )
        return self.async_show_form(
            step_id="user",
            data_schema=self.add_suggested_values_to_schema(schema, user_input),
            errors=errors,
        )

    async def async_step_history(
        self, user_input: dict[str, Any] | None = None
    ) -> ConfigFlowResult:
        """Optionally import an old chaac.db before the receiver starts."""
        errors: dict[str, str] = {}
        if user_input is not None:
            if file_id := user_input.get(CONF_HISTORY_DB):
                dest = Path(self.hass.config.path(".storage", f"chaac_import_{self.flow_id}.db"))
                try:
                    summary = await self.hass.async_add_executor_job(
                        _save_history_db, self.hass, file_id, dest
                    )
                except (InvalidHistoryDb, ValueError, OSError):
                    errors["base"] = "invalid_history"
                else:
                    self._data[CONF_HISTORY_DB] = str(dest)
                    _LOGGER.info("History to import: %s", summary)
            if not errors:
                return self.async_create_entry(title="Chaac receiver", data=self._data)

        return self.async_show_form(
            step_id="history",
            data_schema=vol.Schema(
                {
                    vol.Optional(CONF_HISTORY_DB): FileSelector(
                        FileSelectorConfig(accept=".db,.sqlite,.sqlite3")
                    )
                }
            ),
            errors=errors,
        )

    @staticmethod
    @callback
    def async_get_options_flow(config_entry) -> ChaacOptionsFlow:
        return ChaacOptionsFlow()


class ChaacOptionsFlow(OptionsFlow):
    async def async_step_init(
        self, user_input: dict[str, Any] | None = None
    ) -> ConfigFlowResult:
        if user_input is not None:
            return self.async_create_entry(data=user_input)

        options = self.config_entry.options
        receiver = getattr(self.config_entry, "runtime_data", None)
        pending = sorted(getattr(receiver, "pending", set()))
        approved = options.get(CONF_APPROVED, [])
        station_choices = sorted({*approved, *(uid_str(uid) for uid in pending)})

        schema = vol.Schema(
            {
                vol.Required(
                    CONF_AUTO_ADD, default=options.get(CONF_AUTO_ADD, DEFAULT_AUTO_ADD)
                ): BooleanSelector(),
                vol.Required(CONF_APPROVED, default=approved): SelectSelector(
                    SelectSelectorConfig(
                        options=station_choices, multiple=True, custom_value=True
                    )
                ),
                vol.Required(
                    CONF_UNAVAILABLE_AFTER,
                    default=options.get(CONF_UNAVAILABLE_AFTER, DEFAULT_UNAVAILABLE_AFTER),
                ): NumberSelector(
                    NumberSelectorConfig(
                        min=2, max=60, step=1, unit_of_measurement="min",
                        mode=NumberSelectorMode.BOX,
                    )
                ),
            }
        )
        return self.async_show_form(step_id="init", data_schema=schema)
