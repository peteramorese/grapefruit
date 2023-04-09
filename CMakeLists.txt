cmake_minimum_required(VERSION 3.12.1)

project(task_planner)

find_package (Eigen3 3.3 REQUIRED NO_MODULE)

include(ExternalProject)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

#set(CMAKE_INSTALL_LOCAL_ONLY true)

#add_subdirectory(dependencies/yaml-cpp)
#add_subdirectory(dependencies/eigen-3.4.0)

ExternalProject_Add(yaml
    PREFIX yaml
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/dependencies/yaml-cpp
    BINARY_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    STEP_TARGETS build
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/yaml
)

#ExternalProject_Add(eigen
#    PREFIX eigen
#    SOURCE_DIR ${CMAKE_SOURCE_DIR}/dependencies/eigen-3.4.0
#    BINARY_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
#    CMAKE_ARGS
#        -DCMAKE_INSTALL_PREFIX=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/eigen
#)


set(TP_DEPENDENCY_INCLUDE_DIRS 
    ${CMAKE_SOURCE_DIR}/dependencies/yaml-cpp
    ${EIGEN3_INCLUDE_DIRS}
)
set(TP_LIBRARY_DIR ${CMAKE_SOURCE_DIR}/library)

add_subdirectory(library)

add_subdirectory(examples)
add_subdirectory(src)
add_subdirectory(test)
