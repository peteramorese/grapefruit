
add_executable(test_preference_planner_grid_robot_heuristic test_preference_planner_grid_robot_heuristic.cpp)
target_link_libraries(test_preference_planner_grid_robot_heuristic 
	LexSetClass
	GraphClass
	ConditionClass
	StateClass
	SymbSearchClass
	TransitionSystemClass
	BenchmarkClass
	)

add_executable(test_symbolic_methods test_symbolic_methods.cpp)
target_link_libraries(test_symbolic_methods 
	LexSetClass
	GraphClass
	ConditionClass
	StateClass
	SymbSearchClass
	TransitionSystemClass
	BenchmarkClass
	OrderedPlannerClass
	)

add_executable(graph_test graph_test.cpp)
target_link_libraries(graph_test 
	GraphClass
	)

add_executable(test_label_logic test_label_logic.cpp)
target_link_libraries(test_label_logic
	StateClass
	ConditionClass
	TransitionSystemClass
	GraphClass
	)

add_executable(test_risk_values test_risk_values.cpp)
target_link_libraries(test_risk_values 
	GameClass
	StratSynthesisClass
	)

add_executable(test_create_pareto_front test_create_pareto_front.cpp)
target_link_libraries(test_create_pareto_front
	OrderedPlannerClass
	)