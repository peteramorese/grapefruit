cmake_minimum_required(VERSION 3.10.2)

project(task_planner)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_subdirectory(include)
#add_subdirectory(pref_include)
#add_subdirectory(examples)



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
add_executable(graph_eval examples/graph_eval.cpp)
target_include_directories(graph_eval PUBLIC include)
target_link_libraries(graph_eval 
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

# Preference Planning Examples:
add_executable(lex_set_comp examples/lex_set_comp.cpp)
target_include_directories(lex_set_comp PUBLIC include) #pref_include
target_link_libraries(lex_set_comp 
	LexSetClass
	)

add_executable(preference_planner_manipulator src/preference_planner_manipulator.cpp)
target_include_directories(preference_planner_manipulator PUBLIC include) #pref_include
target_link_libraries(preference_planner_manipulator 
	LexSetClass
	GraphClass
	ConditionClass
	StateSpaceClass
	StateClass
	SymbSearchClass
	)
add_executable(preference_planner_grid_robot src/preference_planner_grid_robot.cpp)
target_include_directories(preference_planner_grid_robot PUBLIC include) #pref_include
target_link_libraries(preference_planner_grid_robot 
	LexSetClass
	GraphClass
	ConditionClass
	StateSpaceClass
	StateClass
	SymbSearchClass
	)
