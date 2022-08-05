
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

add_executable(risk_avoid_rogue_agent risk_avoid_rogue_agent.cpp)
target_link_libraries(risk_avoid_rogue_agent 
	LexSetClass
	GraphClass
	ConditionClass
	StateClass
	TransitionSystemClass
	GameClass
	StratSynthesisClass
	MockGamePlayClass
	)

add_executable(risk_avoid_grid_robot risk_avoid_grid_robot.cpp)
target_link_libraries(risk_avoid_grid_robot
	LexSetClass
	GraphClass
	ConditionClass
	StateClass
	TransitionSystemClass
	GameClass
	StratSynthesisClass
	MockGamePlayClass
	)

add_executable(ordered_planner_grid_robot ordered_planner_grid_robot.cpp)
target_link_libraries(ordered_planner_grid_robot 
	TransitionSystemClass
	OrderedPlannerClass
	BenchmarkClass
	WriteToFileClass
	)