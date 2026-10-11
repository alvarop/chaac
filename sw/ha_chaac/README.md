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

When adding the receiver, the setup dialog has an optional step to upload a
`chaac.db` from the old system. Take a fresh copy of it right before adding
the integration (on the old Pi, the Flask app serves one at `/zipdb`). Rain
that falls between the copy and the first live packet is not recorded.

The import runs once, before the receiver starts listening:

- Stations in the DB get their devices up front, named as in the DB
  (`devices` table), so entities are e.g. `sensor.garden_temperature`.
- Hourly history (`hour_samples`) goes into each sensor's own long-term
  statistics, so old and new data are one series. Gust is skipped: the old
  logger always stored 0.
- Each rain meter starts at the station's imported rain total. HA hasn't
  recorded the meter yet, so it takes that as its starting point rather than
  rain. The recorder starts the live rain sum at 0, so once the first live
  5-minute statistics exist the integration shifts them up by the imported
  total (same as *Adjust sum* in Developer tools > Statistics). Both the
  reading and the sum are then continuous across the switch.

Try it on a throwaway HA first: imported statistics are hard to remove.
Uploads are limited to 100 MB.

## Tests

The protocol decoder and the chaac.db reader have no HA dependency:

```
sw/venv/bin/python -m pytest sw/ha_chaac/tests
```
