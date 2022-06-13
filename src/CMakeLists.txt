
add_executable(preference_planner_manipulator preference_planner_manipulator.cpp)
target_link_libraries(preference_planner_manipulator
	LexSetClass
	GraphClass
	ConditionClass
	StateClass
	SymbSearchClass
	TransitionSystemClass
	BenchmarkClass
	)

add_executable(preference_planner_grid_robot preference_planner_grid_robot.cpp)
target_link_libraries(preference_planner_grid_robot
	LexSetClass
	GraphClass
	ConditionClass
	StateClass
	SymbSearchClass
	TransitionSystemClass
	BenchmarkClass
	)


add_executable(risk_mitigation_grid_robot risk_mitigation_grid_robot.cpp)
target_link_libraries(risk_mitigation_grid_robot
	LexSetClass
	GraphClass
	ConditionClass
	StateClass
	SymbSearchClass
	TransitionSystemClass
	BenchmarkClass
	)

add_executable(risk_avoid_manipulator risk_avoid_manipulator.cpp)
target_link_libraries(risk_avoid_manipulator
	LexSetClass
	GraphClass
	ConditionClass
	StateClass
	TransitionSystemClass
	GameClass
	StratSynthesisClass
	MockGamePlayClass
	)
