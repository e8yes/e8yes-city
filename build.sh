#!/bin/bash

mkdir --p bin
pushd bin
cmake ..
make -j `nproc`
find . -type f -name '*_test' -exec {} \;
popd
