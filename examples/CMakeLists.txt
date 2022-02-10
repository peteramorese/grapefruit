#set(base_include_dir ${CMAKE_SOURCE_DIR}/include)
#set(pref_include_dir ${CMAKE_SOURCE_DIR}/pref_include)

add_executable(file_read_example file_read.cpp)
target_include_directories(file_read_example PUBLIC ${base_include_dir})
target_link_libraries(file_read_example 
	GraphClass
	)

add_executable(add_delete_nodes_example add_delete_nodes.cpp)
target_include_directories(add_delete_nodes_example PUBLIC ${base_include_dir})
target_link_libraries(add_delete_nodes_example 
	GraphClass
	)

add_executable(graph_test graph_test.cpp)
target_include_directories(graph_test PUBLIC ${base_include_dir})
target_link_libraries(graph_test 
	GraphClass
	)

add_executable(graph_eval graph_eval.cpp)
target_include_directories(graph_eval PUBLIC ${base_include_dir})
target_link_libraries(graph_eval 
	GraphClass
	)

add_executable(create_transition_system create_transition_system.cpp)
target_include_directories(create_transition_system PUBLIC ${base_include_dir})
target_link_libraries(create_transition_system 
	StateSpaceClass
	StateClass
	ConditionClass
	TransitionSystemClass
	AstarClass
	GraphClass
	)

add_executable(lex_set_comp lex_set_comp.cpp)
target_include_directories(lex_set_comp PUBLIC ${base_include_dir} ${pref_include_dir}) 
target_link_libraries(lex_set_comp
	LexSetClass
	)

#add_executable(preference_planner_manipulator src/preference_planner_manipulator.cpp)
#target_include_directories(preference_planner_manipulator PUBLIC ${base_include_dir} pref_include_dir) 
#target_link_libraries(preference_planner_manipulator
#	LexSetClass
#	GraphClass
#	ConditionClass
#	StateSpaceClass
#	StateClass
#	SymbSearchClass
#	TransitionSystemClass
#	)
#add_executable(preference_planner_grid_robot src/preference_planner_grid_robot.cpp)
#target_include_directories(preference_planner_grid_robot PUBLIC ${base_include_dir} pref_include_dir) 
#target_link_libraries(preference_planner_grid_robot
#	LexSetClass
#	GraphClass
#	ConditionClass
#	StateSpaceClass
#	StateClass
#	SymbSearchClass
#	TransitionSystemClass
#	)

