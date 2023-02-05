cmake_minimum_required(VERSION 3.12.1)

project(task_planner)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
#add_compile_options(-lstdc++fs)

#set(base_include_dir ${CMAKE_SOURCE_DIR}/include)
#set(pref_include_dir ${CMAKE_SOURCE_DIR}/pref_include)

#set(base_include_dir ${CMAKE_CURRENT_LIST_DIR}/include)
#set(pref_include_dir ${CMAKE_CURRENT_LIST_DIR}/pref_include)

add_subdirectory(dependencies/yaml-cpp)

set(YAML_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/dependencies/yaml-cpp)
set(TP_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/src)

#include_directories(PUBLIC include/headers)
add_subdirectory(src)
#add_subdirectory(src)
#add_subdirectory(test/src)
add_subdirectory(examples/core)
add_subdirectory(examples/graph_search)
