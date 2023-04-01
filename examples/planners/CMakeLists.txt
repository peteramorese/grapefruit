add_executable(DeterministicTaskPlannerExample DeterministicTaskPlannerExample.cpp)
target_link_libraries(DeterministicTaskPlannerExample PUBLIC
	Condition
	StateSpace
	State
	TransitionSystem
	DeterministicTaskPlanner
	Manipulator
)
target_include_directories(DeterministicTaskPlannerExample PUBLIC
	${TP_LIBRARY_DIR}
)

add_executable(BOPreferencePlannerExample BOPreferencePlannerExample.cpp)
target_link_libraries(BOPreferencePlannerExample PUBLIC
	Condition
	StateSpace
	State
	TransitionSystem
	GridWorldAgent
	
)
target_include_directories(BOPreferencePlannerExample PUBLIC
	${TP_LIBRARY_DIR}
)

add_executable(MOPreferencePlannerExample MOPreferencePlannerExample.cpp)
target_link_libraries(MOPreferencePlannerExample PUBLIC
	Condition
	StateSpace
	State
	TransitionSystem
	GridWorldAgent
	
)
target_include_directories(MOPreferencePlannerExample PUBLIC
	${TP_LIBRARY_DIR}
)

add_executable(MCTGExample MCTGExample.cpp)
target_link_libraries(MCTGExample PUBLIC
	Condition
	StateSpace
	State
	TransitionSystem
	GridWorldAgent
)
target_include_directories(MCTGExample PUBLIC
	${TP_LIBRARY_DIR}
)
