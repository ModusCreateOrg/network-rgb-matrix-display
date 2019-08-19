
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
TMP_DIR="$DIR/tmp"

## Install core libs
apt-get update -y
apt-get upgrade -y
apt-get dist-upgrade -y
apt-get install g++ vim make libsdl2-dev libsdl2-image-dev rsync git -y

TMP_DIR="$(mktemp -d)"
cd "$TMP_DIR"

#BOOST
echo "Downloading Lib Boost 1.70.0 ...."
wget -c https://dl.bintray.com/boostorg/release/1.70.0/source/boost_1_70_0.tar.gz -O - | tar -xz

echo "Downloading CMake..."
wget -c https://github.com/Kitware/CMake/releases/download/v3.15.1/cmake-3.15.1.tar.gz -O - | tar -xz


echo "Compiling Boost..."
cd "$TMP_DIR/boost_1_70_0/"
./bootstrap.sh
./b2 -j4 --with-iostreams --with-thread --with-headers threading=multi install

cd "$TMP_DIR/cmake-3.15.1"
./configure
make -j 4 install
cd "$DIR"

rm -rf "$TMP_DIR"

# RGB Matrix server stuff
git clone https://github.com/ModusCreateOrg/network-rgb-matrix-display.git --recursive-submodules

"$DIR/network-rgb-matrix-display/bin/mkbuild.sh"

echo "Done"
#shutdown -h now
