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

glslc sim/shaders/advect.comp -o sim/shaders/advect.comp.spv
glslc sim/shaders/divergence.comp -o sim/shaders/divergence.comp.spv
glslc sim/shaders/jacobi.comp -o sim/shaders/jacobi.comp.spv
glslc sim/shaders/project.comp -o sim/shaders/project.comp.spv
glslc sim/shaders/splat.comp -o sim/shaders/splat.comp.spv
glslc sim/shaders/fluid.vert -o sim/shaders/fluid.vert.spv
glslc sim/shaders/fluid.frag -o sim/shaders/fluid.frag.spv
