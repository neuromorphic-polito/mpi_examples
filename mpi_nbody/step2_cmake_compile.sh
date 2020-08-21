#!/usr/bin/env bash
cwd=$(pwd)
cmake="cmake"
cmake_generator="CodeBlocks - Unix Makefiles"
cmake_path="cmake-build-debug"

cd $cmake_path
cwd2=$(pwd)
$cmake --build $cwd2 --target all -- -j 2
cd $cwd
