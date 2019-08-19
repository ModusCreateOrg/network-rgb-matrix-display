#!/usr/bin/env bash
#
# kill-matrix-server.sh
#
# Kills the LED matrix server

# Set bash unofficial strict mode http://redsymbol.net/articles/unofficial-bash-strict-mode/
set -euo pipefail
IFS=$'\n\t'

# Set DEBUG to true for enhanced debugging: run prefixed with "DEBUG=true"
${DEBUG:-false} && set -vx
# Credit to https://stackoverflow.com/a/17805088
# and http://wiki.bash-hackers.org/scripting/debuggingtips
export PS4='+(${BASH_SOURCE}:${LINENO}): ${FUNCNAME[0]:+${FUNCNAME[0]}(): }'

PIDS=$(pgrep -i matrix-server)
for pid in $PIDS; do 
    if [[ -n "${pid}" ]]; then
        kill "${pid}"
    fi
done
