#!/usr/bin/env bash
#
# install-service.sh
#
# Install the systemd service that supports the LED matrix display

# Set bash unofficial strict mode http://redsymbol.net/articles/unofficial-bash-strict-mode/
set -euo pipefail
IFS=$'\n\t'

# Set DEBUG to true for enhanced debugging: run prefixed with "DEBUG=true"
${DEBUG:-false} && set -vx
# Credit to https://stackoverflow.com/a/17805088
# and http://wiki.bash-hackers.org/scripting/debuggingtips
export PS4='+(${BASH_SOURCE}:${LINENO}): ${FUNCNAME[0]:+${FUNCNAME[0]}(): }'

# Credit to http://stackoverflow.com/a/246128/424301
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
BASE_DIR="$DIR/.."
SERVER_DIR="$BASE_DIR/server"
CFG="$SERVER_DIR/pidisplay.service"

cp "$CFG" /etc/systemd/system/pidisplay.service
systemctl daemon-reload
systemctl enable pidisplay
systemctl start pidisplay
