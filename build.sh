#!/bin/bash

set -e

if [ "$1" = "--recompile_proto" ]
then
    echo "Compiling protocol buffer definitions..."
    protoc --cpp_out=. --python_out=. --proto_path=. `find . -name '*.proto'`
fi

# Builds and tests C++ binaries.
mkdir -p bin
pushd bin
cmake ..
make -j `nproc`
tests=$(find . -type f -name '*_test')
for test_suite in $tests
do
    test_suite_name=$(basename $test_suite)
    echo "$(tput setaf 2)$test_suite_name - Running$(tput setaf 0)"
    $test_suite
    echo "$(tput setaf 2)$test_suite_name - OK$(tput setaf 0)"
    echo "==================================================="
done
popd

# Tests Python modules.
python3 -m intermediate_representation.catmul_rom_test
python3 -m procedural.probing.population_test
python3 -m procedural.street.curve_test
python3 -m procedural.street.intersection_area_test
python3 -m procedural.street.ir_street_test
