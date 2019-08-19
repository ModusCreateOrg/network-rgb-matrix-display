
#!/usr/bin/env bash
#
# install.sh
#
# Install the Network RGB Matrix Display

# Set bash unofficial strict mode http://redsymbol.net/articles/unofficial-bash-strict-mode/
set -euo pipefail
IFS=$'\n\t'

# Set DEBUG to true for enhanced debugging: run prefixed with "DEBUG=true"
${DEBUG:-false} && set -vx
# Credit to https://stackoverflow.com/a/17805088
# and http://wiki.bash-hackers.org/scripting/debuggingtips
export PS4='+(${BASH_SOURCE}:${LINENO}): ${FUNCNAME[0]:+${FUNCNAME[0]}(): }'

DIR="/root"

echo "***** Install core libraries"
apt-get update -y
apt-get upgrade -y
apt-get dist-upgrade -y
apt-get install g++ vim make libsdl2-dev libsdl2-image-dev rsync git -y

TMP_DIR="$(mktemp -d)"
function finish {
    RETCODE=$?
    if [[ "$RETCODE" -ne 0 ]]; then
	echo "***** Exiting with return code $RETCODE. Temp dir $TMP_DIR contents:"
        ls -l "$DIR"  
    fi
    rm -rf "$TMP_DIR"
    return "$RETCODE"
}
trap finish EXIT

cd "$TMP_DIR"

echo "***** Downloading Lib Boost 1.70.0..."
BOOST_DIR="$TMP_DIR/boost_1_70_0"
wget -c https://dl.bintray.com/boostorg/release/1.70.0/source/boost_1_70_0.tar.gz -O - | tar -xz

echo "***** Downloading CMake..."
wget -c https://github.com/Kitware/CMake/releases/download/v3.15.1/cmake-3.15.1.tar.gz -O - | tar -xz

echo "***** Compiling Boost..."
cd "$BOOST_DIR"
./bootstrap.sh
./b2 -j4 --with-iostreams --with-thread --with-headers threading=multi install
# This directory is over 700MB after compiling, remove it to save /tmp space
cd "$TMP_DIR"
rm -rf "$BOOST_DIR"

echo "***** Compiling CMake..."
cd "$TMP_DIR/cmake-3.15.1"
./configure
make -j 4 install
cd "$DIR"

echo "**** Cloning network-rgb-matrix-display..."
# RGB Matrix server stuff
git clone https://github.com/ModusCreateOrg/network-rgb-matrix-display.git --recurse-submodules

"$DIR/network-rgb-matrix-display/bin/mkbuild.sh"

echo "**** Installing service..."

"$DIR/network-rgb-matrix-display/bin/install-service.sh"

echo "**** Done"
