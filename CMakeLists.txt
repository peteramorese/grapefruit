cmake_minimum_required(VERSION 3.10.2)

project(task_planner)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(base_include_dir ${CMAKE_SOURCE_DIR}/include)
set(pref_include_dir ${CMAKE_SOURCE_DIR}/pref_include)

add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(pref_include)
add_subdirectory(examples)



