cmake_minimum_required(VERSION 3.10.2)

project(task_planner)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#set(base_include_dir ${CMAKE_SOURCE_DIR}/include)
#set(pref_include_dir ${CMAKE_SOURCE_DIR}/pref_include)

#set(base_include_dir ${CMAKE_CURRENT_LIST_DIR}/include)
#set(pref_include_dir ${CMAKE_CURRENT_LIST_DIR}/pref_include)

include_directories(PUBLIC include/headers)
add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(test/src)
add_subdirectory(examples)

#add_executable(graph_test examples/graph_test.cpp)
##target_include_directories(graph_test PUBLIC ${base_include_dir})
#target_include_directories(graph_test PUBLIC include/headers)
#target_link_libraries(graph_test 
#	GraphClass
#	)


