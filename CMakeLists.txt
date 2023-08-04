cmake_minimum_required(VERSION 3.20)

project(grapefruit)

find_package (Eigen3 3.3.8 REQUIRED NO_MODULE)
find_package(Threads REQUIRED)

include(ExternalProject)

if(NOT DEFINED GF_BUILD_EXECUTABLES)
    option(GF_BUILD_EXECUTABLES "Build executables (set to OFF for building just the library target)" ON)
endif()

if(NOT DEFINED ENABLE_MATRIX_CHECKS)
    option(ENABLE_MATRIX_CHECKS "Enable matrix pos-def/pos-semi-def checks (Eigen >= 3.3.8)" ON)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

message("CMAKE_RUNTIME_OUTPUT_DIRECTORY: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message("CMAKE_LIBRARY_OUTPUT_DIRECTORY: ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

# Dependencies
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/yaml-cpp)

set(GF_DEPENDENCY_INCLUDE_DIRS 
    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/yaml-cpp/include
    ${EIGEN3_INCLUDE_DIRS}
)
set(GF_LIBRARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/grapefruit)

add_subdirectory(grapefruit)

if(GF_BUILD_EXECUTABLES)
    add_subdirectory(examples)
    add_subdirectory(src)
    add_subdirectory(test)
endif()

if(GF_ENABLE_MATRIX_CHECKS)
    add_definitions(-DGF_CHECK_MATRIX_POS_DEF)
endif()
