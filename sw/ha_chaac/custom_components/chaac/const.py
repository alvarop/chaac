"""Constants for the Chaac integration."""

DOMAIN = "chaac"

CONF_SERIAL_PORT = "serial_port"
# Create devices for any station UID heard (otherwise only approved UIDs)
CONF_AUTO_ADD = "auto_add"
CONF_APPROVED = "approved_stations"
CONF_UNAVAILABLE_AFTER = "unavailable_after"

DEFAULT_AUTO_ADD = True
# Stations send once a minute
DEFAULT_UNAVAILABLE_AFTER = 5  # minutes

BAUD_RATE = 115200

EVENT_RESET = "chaac_reset"


def signal_reading(entry_id: str) -> str:
    return f"{DOMAIN}_{entry_id}_reading"


def signal_new_station(entry_id: str) -> str:
    return f"{DOMAIN}_{entry_id}_new_station"


def signal_tick(entry_id: str) -> str:
    return f"{DOMAIN}_{entry_id}_tick"


def uid_str(uid: int) -> str:
    return f"{uid:08X}"
