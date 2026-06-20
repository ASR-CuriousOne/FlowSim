#!/bin/bash

BUILD_TYPE="Debug"
BUILD_DIR="build/debug"

if [ "$1" == "release" ] || [ "$1" == "Release" ]; then
    BUILD_TYPE="Release"
    BUILD_DIR="build/release"
    echo "--- Configuring for Release Mode ---"
else
    echo "--- Configuring for Debug Mode ---"
fi

mkdir -p $BUILD_DIR

cmake -B $BUILD_DIR -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=$BUILD_TYPE
cmake --build $BUILD_DIR
