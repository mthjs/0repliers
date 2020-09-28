#!/usr/bin/env bash

mkdir -p build
pushd build > /dev/null
cmake .. \
   -D CMAKE_C_COMPILER="$C_COMPILER" \
   -D CMAKE_CXX_COMPILER="$CXX_COMPILER"
