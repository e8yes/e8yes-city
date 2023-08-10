cmake_minimum_required(VERSION 3.10)

project(e8citydll)

find_package(Boost REQUIRED COMPONENTS graph log unit_test_framework)
find_package(CGAL REQUIRED)
find_package(Eigen3 REQUIRED NO_MODULE)
find_package(Protobuf REQUIRED)
find_package(pybind11 REQUIRED)

set(CGAL_DO_NOT_WARN_ABOUT_CMAKE_BUILD_TYPE TRUE)
set(PYBIND11_PYTHON_VERSION 3.11)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Release)

if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -fPIC -fsanitize=address -Og")
    set(LINK_FLAGS "-fsanitize=address")
elseif(${CMAKE_BUILD_TYPE} STREQUAL Release)
    set(CMAKE_CXX_FLAGS "-fPIC -Ofast -flto=auto")
    set(LINK_FLAGS "-Ofast -flto=auto")
endif()

set(SRCS
    intermediate_representation/city.pb.cc
    intermediate_representation/compass.pb.cc
    intermediate_representation/curve.pb.cc
    intermediate_representation/intersection.pb.cc
    intermediate_representation/segment.pb.cc
    intermediate_representation/space.pb.cc
    intermediate_representation/street.pb.cc
    procedural/probing/topology/definition.cpp
    procedural/probing/topology/edge_set.cpp
    procedural/probing/topology/init.cpp
    procedural/probing/topology/mutation_efficiency.cpp
    procedural/probing/topology/mutation_regularity.cpp
    procedural/probing/topology/objective_efficiency.cpp
    procedural/probing/topology/objective_regularity.cpp
    procedural/probing/topology/optimize_efficiency.cpp
    procedural/probing/topology/optimize_regularity.cpp
    procedural/probing/topology/sampler.cpp
    procedural/probing/topology/topology.cpp)
set(PYBIND_SRCS
    procedural/probing/topology/pybind.cpp
    pywrap.cpp)
set(LIBS
    Boost::graph
    Boost::log
    CGAL::CGAL
    Eigen3::Eigen
    ${Protobuf_LIBRARIES})

# Main Pybind11 module.
pybind11_add_module(${PROJECT_NAME} ${SRCS} ${PYBIND_SRCS})
target_compile_definitions(${PROJECT_NAME} PRIVATE VERSION_INFO=${EXAMPLE_VERSION_INFO})
target_include_directories(${PROJECT_NAME} PRIVATE . ${PYBIND11_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBS})

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
add_test(procedural_probing_topology_init_test 
         procedural/probing/topology/init_test.cpp)
add_test(procedural_probing_topology_mutation_efficiency_test 
         procedural/probing/topology/mutation_efficiency_test.cpp)
add_test(procedural_probing_topology_mutation_regularity_test 
         procedural/probing/topology/mutation_regularity_test.cpp)
add_test(procedural_probing_topology_objective_efficiency_test 
         procedural/probing/topology/objective_efficiency_test.cpp)
add_test(procedural_probing_topology_objective_regularity_test 
         procedural/probing/topology/objective_regularity_test.cpp)
add_test(procedural_probing_topology_optimize_efficiency_test 
         procedural/probing/topology/optimize_efficiency_test.cpp)
add_test(procedural_probing_topology_optimize_regularity_test 
         procedural/probing/topology/optimize_regularity_test.cpp)
add_test(procedural_probing_topology_sampler_test 
         procedural/probing/topology/sampler_test.cpp)
