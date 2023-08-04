add_executable(DeterministicTaskPlannerExample DeterministicTaskPlannerExample.cpp)
target_link_libraries(DeterministicTaskPlannerExample PUBLIC
	Core
	Models
	Planners
)
target_include_directories(DeterministicTaskPlannerExample PUBLIC
	${GF_LIBRARY_DIR}
)

add_executable(BOPreferencePlannerExample BOPreferencePlannerExample.cpp)
target_link_libraries(BOPreferencePlannerExample PUBLIC
	Core
	Models
	Planners
)
target_include_directories(BOPreferencePlannerExample PUBLIC
	${GF_LIBRARY_DIR}
)

add_executable(MOPreferencePlannerExample MOPreferencePlannerExample.cpp)
target_link_libraries(MOPreferencePlannerExample PUBLIC
	Core
	Models
	Planners
)
target_include_directories(MOPreferencePlannerExample PUBLIC
	${GF_LIBRARY_DIR}
)

add_executable(MCTGExample MCTGExample.cpp)
target_link_libraries(MCTGExample PUBLIC
	Core
	Models
	Planners
)
target_include_directories(MCTGExample PUBLIC
	${GF_LIBRARY_DIR}
)
