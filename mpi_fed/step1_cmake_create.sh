#!/usr/bin/env bash
cwd=$(pwd)
cmake="cmake"
cmake_generator="CodeBlocks - Unix Makefiles"
cmake_path="cmake-build-debug"

rm -rf $cmake_path; mkdir -p $cmake_path
cd $cmake_path
cwd2=$(pwd)
$cmake -DCMAKE_BUILD_TYPE=Debug -G "$cmake_generator" $cwd
cd $cwd
