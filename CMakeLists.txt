cmake_minimum_required(VERSION 3.10.2)

project(task_planner)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#add_executable(${PROJECT_NAME} src/test1.cpp)
add_executable(${PROJECT_NAME} src/tp_test.cpp)
add_subdirectory(include)
#add_subdirectory(examples)
target_include_directories(${PROJECT_NAME} PUBLIC include)
link_directories(${PROJECT_NAME} PUBLIC include)
#target_link_libraries(${PROJECT_NAME} EdgeClass AstarClass StateClass TransitionSystemClass)
target_link_libraries(${PROJECT_NAME} 
	EdgeClass 
	AstarClass 
	StateSpaceClass 
	StateClass 
	ConditionClass
        TransitionSystemClass
	AutomatonClass
	)	

add_executable(file_read_example examples/file_read.cpp)
target_include_directories(file_read_example PUBLIC include)
target_link_libraries(file_read_example 
	AutomatonClass
	)
