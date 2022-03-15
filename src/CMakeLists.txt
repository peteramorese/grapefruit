#set(base_include_dir ${CMAKE_SOURCE_DIR}/include)
#set(pref_include_dir ${CMAKE_SOURCE_DIR}/pref_include)

add_executable(preference_planner_manipulator preference_planner_manipulator.cpp)
target_include_directories(preference_planner_manipulator PUBLIC ${base_include_dir} ${pref_include_dir}) 
target_link_libraries(preference_planner_manipulator
	LexSetClass
	GraphClass
	ConditionClass
	StateSpaceClass
	StateClass
	SymbSearchClass
	TransitionSystemClass
	BenchmarkClass
	)

add_executable(preference_planner_grid_robot preference_planner_grid_robot.cpp)
target_include_directories(preference_planner_grid_robot PUBLIC ${base_include_dir} ${pref_include_dir}) 
target_link_libraries(preference_planner_grid_robot
	LexSetClass
	GraphClass
	ConditionClass
	StateSpaceClass
	StateClass
	SymbSearchClass
	TransitionSystemClass
	BenchmarkClass
	)

