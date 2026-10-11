# Chaac for Home Assistant

Custom integration that reads a Chaac USB LoRa receiver (`usb_lora` firmware)
over serial and creates one HA device per weather station UID.

- Weather: temperature, humidity, pressure (station/absolute), wind speed, gust,
  direction, rain total (`total_increasing`, so HA long-term statistics work).
- Diagnostics: battery and solar voltage, RSSI, SNR, last packet time.
- Readings the station flags as failed (e.g. pressure `INT16_MIN`) become
  `unknown`. When a `chaac_wxt` station sends a packet without fresh Vaisala
  data, wind is dropped too because the firmware repeats the old wind values.
- Station resets fire a `chaac_reset` event with the station UID.

## Install

Copy `custom_components/chaac` into the HA config directory
(`/config/custom_components/chaac`) and restart, then add the *Chaac Weather
Station* integration and pick the receiver's `/dev/serial/by-id/...` port.

HACS needs `custom_components/` at the repository root, so installing from this
monorepo through HACS won't work as-is.

## Importing old data

`scripts/import_chaac_db.py` copies `hour_samples` from a `chaac.db` into the
Chaac sensors' long-term statistics, so the graphs go back before the switch.
Run it once the integration has recorded at least one hour: it only imports
hours before the first hour HA already has, and never overwrites.

```
sw/venv/bin/python sw/ha_chaac/scripts/import_chaac_db.py --db chaac.db \
    --url ws://homeassistant.local:8123 --token-file ~/.config/ha_token \
    --rename-devices --dry-run
```

Drop `--dry-run` to import. Needs the `websockets` package. Try it on a
throwaway HA first: imported statistics are hard to remove.

## Tests

The protocol decoder has no HA dependency:

```
sw/venv/bin/python -m pytest sw/ha_chaac/tests
```
