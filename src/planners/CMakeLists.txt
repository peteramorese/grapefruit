add_executable(BOPartialSatisfactionPreferencePlanner BOPartialSatisfactionPreferencePlanner.cpp)
target_link_libraries(BOPartialSatisfactionPreferencePlanner PUBLIC
	Core
	Models
	Planners
)
target_include_directories(BOPartialSatisfactionPreferencePlanner PUBLIC
	${GF_LIBRARY_DIR}
)

add_executable(BOActionCostPreferencePlanner BOActionCostPreferencePlanner.cpp)
target_link_libraries(BOActionCostPreferencePlanner PUBLIC
	Core
	Models
	Planners
)
	
add_executable(MOPreferencePlanner MOPreferencePlanner.cpp)
target_link_libraries(MOPreferencePlanner PUBLIC
	Core
	Models
	Planners
)
target_include_directories(MOPreferencePlanner PUBLIC
	${GF_LIBRARY_DIR}
)
