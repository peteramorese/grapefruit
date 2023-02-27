add_executable(BOPartialSatisfactionPreferencePlanner BOPartialSatisfactionPreferencePlanner.cpp)
target_link_libraries(BOPartialSatisfactionPreferencePlanner PUBLIC
	Condition
	StateSpace
	State
	TransitionSystem
	GridWorldAgent
	
)
target_include_directories(BOPartialSatisfactionPreferencePlanner PUBLIC
	${TP_LIBRARY_DIR}
)

add_executable(MOPreferencePlanner MOPreferencePlanner.cpp)
target_link_libraries(MOPreferencePlanner PUBLIC
	Condition
	StateSpace
	State
	TransitionSystem
	GridWorldAgent
	
)
target_include_directories(MOPreferencePlanner PUBLIC
	${TP_LIBRARY_DIR}
)
