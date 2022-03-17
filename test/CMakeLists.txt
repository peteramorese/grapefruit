#set(base_include_dir ${CMAKE_SOURCE_DIR}/include)
#set(pref_include_dir ${CMAKE_SOURCE_DIR}/pref_include)

add_executable(test_preference_planner_grid_robot_heuristic test_preference_planner_grid_robot_heuristic.cpp)
target_include_directories(test_preference_planner_grid_robot_heuristic PUBLIC ${base_include_dir} ${pref_include_dir}) 
target_link_libraries(test_preference_planner_grid_robot_heuristic 
	LexSetClass
	GraphClass
	ConditionClass
	StateSpaceClass
	StateClass
	SymbSearchClass
	TransitionSystemClass
	BenchmarkClass
	)

