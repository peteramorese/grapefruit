add_executable(GraphTest GraphTest.cpp)
target_link_libraries(GraphTest PUBLIC
)
target_include_directories(GraphTest PUBLIC
	${TP_INCLUDE_DIRS}
	#${SPOT_INCLUDE_DIRS}
)

add_executable(YAMLTest YAMLTest.cpp)
target_link_libraries(YAMLTest PUBLIC
	yaml-cpp
)
target_include_directories(YAMLTest PUBLIC
	${TP_INCLUDE_DIRS}
	${YAML_INCLUDE_DIRS}
)


#add_executable(file_read_example file_read.cpp)
#target_link_libraries(file_read_example 
#	GraphClass
#	)

#add_executable(add_delete_nodes_example add_delete_nodes.cpp)
#target_link_libraries(add_delete_nodes_example 
#	GraphClass
#	)
#
#add_executable(graph_eval graph_eval.cpp)
#target_link_libraries(graph_eval 
#	GraphClass
#	)
#
#add_executable(create_transition_system create_transition_system.cpp)
#target_link_libraries(create_transition_system 
#	StateClass
#	ConditionClass
#	TransitionSystemClass
#	GraphClass
#	)
#
#add_executable(create_game create_game.cpp)
##target_include_directories(create_game PUBLIC ../include/headers)
#target_link_libraries(create_game 
#	StateClass
#	ConditionClass
#	TransitionSystemClass
#	GraphClass
#	GameClass
#	)
#
##target_link_libraries(create_game GameClass)
#
#add_executable(lex_set_comp lex_set_comp.cpp)
#target_link_libraries(lex_set_comp
#	LexSetClass
#	)
