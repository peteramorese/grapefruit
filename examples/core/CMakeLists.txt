add_executable(GraphExample GraphExample.cpp)
target_link_libraries(GraphExample PUBLIC
)
target_include_directories(GraphExample PUBLIC
	${TP_INCLUDE_DIRS}
	#${SPOT_INCLUDE_DIRS}
)

add_executable(YAMLExample YAMLExample.cpp)
target_link_libraries(YAMLExample PUBLIC
	yaml-cpp
)
target_include_directories(YAMLExample PUBLIC
	${TP_INCLUDE_DIRS}
	${YAML_INCLUDE_DIRS}
)

add_executable(StateExample StateExample.cpp)
target_link_libraries(StateExample PUBLIC
	State
	StateSpace
	yaml-cpp
)
target_include_directories(StateExample PUBLIC
	${TP_INCLUDE_DIRS}
	${YAML_INCLUDE_DIRS}
)

add_executable(StateSpaceExample StateSpaceExample.cpp)
target_link_libraries(StateSpaceExample PUBLIC
	StateSpace
	yaml-cpp
)
target_include_directories(StateSpaceExample PUBLIC
	${TP_INCLUDE_DIRS}
	${YAML_INCLUDE_DIRS}
)

add_executable(ConditionExample ConditionExample.cpp)
target_link_libraries(ConditionExample PUBLIC
	State
	StateSpace
	Condition
)
target_include_directories(ConditionExample PUBLIC
	${TP_INCLUDE_DIRS}
	${YAML_INCLUDE_DIRS}
)

add_executable(TransitionSystemExample TransitionSystemExample.cpp)
target_link_libraries(TransitionSystemExample PUBLIC
	State
	StateSpace
	Condition
	TransitionSystem
)
target_include_directories(TransitionSystemExample PUBLIC
	${TP_INCLUDE_DIRS}
	${YAML_INCLUDE_DIRS}
)

add_executable(SymbolicProductAutomatonExample SymbolicProductAutomatonExample.cpp)
target_link_libraries(SymbolicProductAutomatonExample PUBLIC
	State
	StateSpace
	Condition
	TransitionSystem
	Manipulator
)
target_include_directories(SymbolicProductAutomatonExample PUBLIC
	${TP_INCLUDE_DIRS}
	${YAML_INCLUDE_DIRS}
)
