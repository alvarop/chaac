# Chaac Logger Setup

## On Raspberry Pi

1. Clone repo to raspberry pi (or scp it over)
2. Run `./support/install_dependencies.sh` to get required software
3. Run `./support/setup.sh` to configure everything and start the server

If you just changed a few files and don't want to go through the whole pipenv process again, you can use `./support/update.sh`. This will just restart the services.

### Monitoring
To monitor packets arriving run:

`sudo tail -f /var/log/supervisor/chaac-logger-app-stdout---supervisor-KsDl4b.log`

Note that the last few characters, `KsDl4b` in this case, change everytime supervisor restarts. It's easy to just tab complete with whatever is there.

To check for errors, just replace stdout for stderr in the command.

## Local Development
Run `pipenv run python chaac_logger.py --port /dev/ttyUSB0 --baud_rate 115200 --db sqlite_db_filename.db`
