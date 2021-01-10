rm -rf build >/dev/null 2>&1
mkdir build
cd build
cmake .. && make -j$(nproc) && ./matrix-server rgb-server.ini


