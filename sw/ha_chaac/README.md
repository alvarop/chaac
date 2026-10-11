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

## Tests

The protocol decoder has no HA dependency:

```
sw/venv/bin/python -m pytest sw/ha_chaac/tests
```
