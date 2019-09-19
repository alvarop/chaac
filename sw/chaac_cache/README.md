# Chaac Cache Setiup

## On Raspberry Pi

1. Clone repo to raspberry pi (or scp it over)
2. Run `./support/install_dependencies.sh` to get required software
3. Run `./support/setup.sh` to configure everything and start the server

**NOTE:** Python files copied over to /var/flaskapp/chaac_cache, so if you edit the ones here, it won't change.

If you just changed a few files and don't want to go through the whole pipenv process again, you can use `./support/update.sh`. This will just rsync the changed files over and restart the services.

## Local Development

### Gunicorn
Run `pipenv run python chaac_cace.py --config config_file.yml`

