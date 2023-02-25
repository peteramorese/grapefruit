cmake_minimum_required(VERSION 3.12.1)

project(task_planner)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(dependencies/yaml-cpp)

set(YAML_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/dependencies/yaml-cpp)
set(TP_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/src)

add_subdirectory(library)
add_subdirectory(examples)
