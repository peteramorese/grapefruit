
add_executable(file_read_example file_read.cpp)
target_link_libraries(file_read_example 
	GraphClass
	)

add_executable(add_delete_nodes_example add_delete_nodes.cpp)
target_link_libraries(add_delete_nodes_example 
	GraphClass
	)

add_executable(graph_test graph_test.cpp)
target_link_libraries(graph_test 
	GraphClass
	)

add_executable(graph_eval graph_eval.cpp)
target_link_libraries(graph_eval 
	GraphClass
	)

add_executable(create_transition_system create_transition_system.cpp)
target_link_libraries(create_transition_system 
	StateClass
	ConditionClass
	TransitionSystemClass
	GraphClass
	)

add_executable(lex_set_comp lex_set_comp.cpp)
target_link_libraries(lex_set_comp
	LexSetClass
	)

add_executable(test_label_logic test_label_logic.cpp)
target_link_libraries(test_label_logic
	StateClass
	ConditionClass
	TransitionSystemClass
	GraphClass
	)
