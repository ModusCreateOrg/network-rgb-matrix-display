
## Install core libs
apt-get update -y
apt-get upgrade -y
apt-get dist-upgrade -y
apt-get install g++ vim make libsdl2-dev libsdl2-image-dev rsync git -y

mkdir -p tmp
cd tmp

#BOOST 
echo "Downloading Lib Boost 1.70.0 ...."
wget -c https://dl.bintray.com/boostorg/release/1.70.0/source/boost_1_70_0.tar.gz -O - | tar -xz

echo "Downloading CMake..."
wget -c https://github.com/Kitware/CMake/releases/download/v3.15.1/cmake-3.15.1.tar.gz -O - | tar -xz


echo "Compiling Boost..."
cd boost_1_70_0/
./bootstrap.sh
./b2 -j4 --with-iostreams --with-thread --with-headers threading=multi install

cd ..

cd cmake-3.15.1 
./configure
make -j 4 install
cd ../..

rm -rf tmp

# RGB Matrix server stuff. TODO: Move to ModusCreateOrg
git clone https://github.com/jaygarcia/network-rgb-matrix-display.git

cd network-rgb-matrix-display/server
./mkbuild.sh

echo "Done"
#shutdown -h now
