#!/bin/bash

# Builds and tests C++ binaries.
mkdir -p bin
pushd bin
cmake ..
make -j `nproc`
find . -type f -name '*_test' -exec {} \;
popd

# Tests Python modules.
python3 -m procedural.probing.population_test
