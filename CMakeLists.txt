cmake_minimum_required(VERSION 3.10)

project(e8citydll)

find_package(Boost REQUIRED COMPONENTS graph log unit_test_framework)
find_package(Eigen3 REQUIRED NO_MODULE)

set(PYBIND11_PYTHON_VERSION 3.11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -fPIC -fsanitize=address -Ofast -flto=auto")
set(LINK_FLAGS "-fsanitize=address -Ofast -flto=auto")

set(SRCS
    procedural/probing/topology/definition.cpp
    procedural/probing/topology/edge_set.cpp
    procedural/probing/topology/mutation.cpp
    procedural/probing/topology/objective.cpp
    procedural/probing/topology/optimizer.cpp
    procedural/probing/topology/sampler.cpp)
set(LIBS
    Boost::graph
    Boost::log
    Eigen3::Eigen)

# Standalone library.
add_library(${PROJECT_NAME}_standalone ${SRCS})
target_include_directories(${PROJECT_NAME}_standalone PUBLIC .)
target_link_libraries(${PROJECT_NAME}_standalone PUBLIC ${LIBS})

# Tests.
function(add_test TEST_NAME SOURCE_FILE_NAME)
    add_executable(${TEST_NAME} ${SOURCE_FILE_NAME})
    target_include_directories(${TEST_NAME} PRIVATE .)
    target_link_libraries(${TEST_NAME}
                          PRIVATE
                          ${PROJECT_NAME}_standalone
                          Boost::unit_test_framework)
endfunction()

add_test(procedural_probing_topology_edge_set_test 
         procedural/probing/topology/edge_set_test.cpp)
add_test(procedural_probing_topology_mutation_test 
         procedural/probing/topology/mutation_test.cpp)
add_test(procedural_probing_topology_objective_test 
         procedural/probing/topology/objective_test.cpp)
add_test(procedural_probing_topology_optimizer_test 
         procedural/probing/topology/optimizer_test.cpp)
add_test(procedural_probing_topology_sampler_test 
         procedural/probing/topology/sampler_test.cpp)
