#!/usr/bin/env bash
#
# get_pi_model.sh
#
# Retrieves the pi model

# Set bash unofficial strict mode http://redsymbol.net/articles/unofficial-bash-strict-mode/
set -euo pipefail
IFS=$'\n\t'

# Set DEBUG to true for enhanced debugging: run prefixed with "DEBUG=true"
${DEBUG:-false} && set -vx
# Credit to https://stackoverflow.com/a/17805088
# and http://wiki.bash-hackers.org/scripting/debuggingtips
export PS4='+(${BASH_SOURCE}:${LINENO}): ${FUNCNAME[0]:+${FUNCNAME[0]}(): }'

MODEL=/sys/firmware/devicetree/base/model
# Not a pi
if [[ ! -f "$MODEL" ]]; then 
	echo -1
else
    cut -d " " -f 3 < /sys/firmware/devicetree/base/model
fi
