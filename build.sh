#!/bin/bash

# Define build directory
BUILD_DIR="build"

# Create build directory if it doesn't exist
if [ ! -d "$BUILD_DIR" ]; then
    mkdir $BUILD_DIR
fi

# Navigate to the build directory
cd $BUILD_DIR

# Run CMake to configure the project
cmake -DCMAKE_TOOLCHAIN_FILE="C:/Users/Matt/vcpkg/scripts/buildsystems/vcpkg.cmake" ..

# Build the project
cmake --build . --config Release
