"""Config flow for Chaac: pick the receiver's serial port."""

from __future__ import annotations

import os
import time
from typing import Any

import serial
import voluptuous as vol

from homeassistant.config_entries import ConfigFlow, ConfigFlowResult, OptionsFlow
from homeassistant.core import callback
from homeassistant.helpers.selector import (
    BooleanSelector,
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
    CONF_SERIAL_PORT,
    CONF_UNAVAILABLE_AFTER,
    DEFAULT_AUTO_ADD,
    DEFAULT_UNAVAILABLE_AFTER,
    DOMAIN,
    uid_str,
)
from .protocol import FrameDecoder, encode_ping, is_pong

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


class ChaacConfigFlow(ConfigFlow, domain=DOMAIN):
    VERSION = 1

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
                    return self.async_create_entry(
                        title="Chaac receiver", data={CONF_SERIAL_PORT: port}
                    )
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
