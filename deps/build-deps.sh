#!/bin/bash

cd $1/deps/freetype
mkdir -p build
cd build
../configure --prefix="$1/libs/freetype"
make -j$(nproc)
make install

cd $1/deps/libpng
mkdir -p build
cd build
../configure --prefix=$1/libs/libpng
make -j$(nproc)
make install