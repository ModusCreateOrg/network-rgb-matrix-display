#!/usr/bin/env bash
rm -rf build/ 2>/dev/null;
mkdir -p build/

cd build;
cmake ..
make -j 4

echo "next steps: cd build/"