cmake_minimum_required(VERSION 3.10.2)

project(task_planner)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#add_executable(${PROJECT_NAME} src/test1.cpp)
#add_executable(${PROJECT_NAME} src/tp_test.cpp)
#target_include_directories(${PROJECT_NAME} PUBLIC include)
#link_directories(${PROJECT_NAME} PUBLIC include)
add_subdirectory(include)
#add_subdirectory(examples)

#target_link_libraries(${PROJECT_NAME} 
#	EdgeClass 
#	GraphClass
#	AstarClass 
#	StateSpaceClass 
#	StateClass 
#	ConditionClass
#        TransitionSystemClass
#	#AutomatonClass
#	)	

add_executable(file_read_example examples/file_read.cpp)
target_include_directories(file_read_example PUBLIC include)
target_link_libraries(file_read_example 
	GraphClass
	)
add_executable(add_delete_nodes_example examples/add_delete_nodes.cpp)
target_include_directories(add_delete_nodes_example PUBLIC include)
target_link_libraries(add_delete_nodes_example 
	#EdgeClass
	GraphClass
	)
add_executable(graph_test examples/graph_test.cpp)
target_include_directories(graph_test PUBLIC include)
target_link_libraries(graph_test 
	GraphClass
	)
add_executable(create_transition_system examples/create_transition_system.cpp)
target_include_directories(create_transition_system PUBLIC include)
target_link_libraries(create_transition_system 
	StateSpaceClass
	StateClass
	ConditionClass
	TransitionSystemClass
	AstarClass
	GraphClass
	)
