#!/usr/bin/env bash
set -e

NPROC=$(nproc)
BUILD_TYPE="Debug"
OS=""
C23_EMBED=""
TRUTH_FFI_INTEGRATION="--truth-integration"

for arg in "$@"; do
    if [ "$arg" = "--clean" ]; then
        CLEAN=true
    elif [ "$arg" = "--emscripten" ]; then
        OS="emscripten"
    elif [ "$arg" = "--use-c23-embed" ]; then
        C23_EMBED="--use-c23-embed"
    elif [ "$arg" = "--no-truth-integration" ]; then
        TRUTH_FFI_INTEGRATION=""
    elif [ "$arg" = "Release" ] || [ "$arg" = "Debug" ] || [ "$arg" = "All" ]; then
        BUILD_TYPE=$arg
    fi
done

if [ "$CLEAN" = true ]; then
    rm -rf build obj
    rm -f th06_*
fi


if [ "$OS" = "emscripten" ]; then
    premake5 --os=emscripten ninja $C23_EMBED $TRUTH_FFI_INTEGRATION
else
    premake5 --cc=clang ninja $C23_EMBED $TRUTH_FFI_INTEGRATION
fi;

premake5 clang ecc

cd build

if [ "$BUILD_TYPE" = "All" ]; then
    ninja -j${NPROC} Debug Release
else
    ninja -j${NPROC} $BUILD_TYPE
fi

if [ "$OS" = "emscripten" ]; then
    cp ../web/th06_${BUILD_TYPE,,}.html ../web/index.html
fi;
