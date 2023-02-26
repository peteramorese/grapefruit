add_executable(GraphExample GraphExample.cpp)
target_link_libraries(GraphExample PUBLIC
)
target_include_directories(GraphExample PUBLIC
	${TP_LIBRARY_DIR}
	#${SPOT_INCLUDE_DIRS}
)

add_executable(DAGExample DAGExample.cpp)
target_link_libraries(DAGExample PUBLIC
)
target_include_directories(DAGExample PUBLIC
	${TP_LIBRARY_DIR}
	#${SPOT_INCLUDE_DIRS}
)

add_executable(AutomatonExample AutomatonExample.cpp)
target_link_libraries(AutomatonExample PUBLIC
	yaml-cpp
)
target_include_directories(AutomatonExample PUBLIC
	${TP_LIBRARY_DIR}
	#${SPOT_INCLUDE_DIRS}
)

add_executable(YAMLExample YAMLExample.cpp)
target_link_libraries(YAMLExample PUBLIC
	yaml-cpp
)
target_include_directories(YAMLExample PUBLIC
	${TP_LIBRARY_DIR}
	${YAML_INCLUDE_DIRS}
)

add_executable(StateExample StateExample.cpp)
target_link_libraries(StateExample PUBLIC
	State
	StateSpace
	yaml-cpp
)
target_include_directories(StateExample PUBLIC
	${TP_LIBRARY_DIR}
	${YAML_INCLUDE_DIRS}
)

add_executable(StateSpaceExample StateSpaceExample.cpp)
target_link_libraries(StateSpaceExample PUBLIC
	StateSpace
	yaml-cpp
)
target_include_directories(StateSpaceExample PUBLIC
	${TP_LIBRARY_DIR}
	${YAML_INCLUDE_DIRS}
)

add_executable(ConditionExample ConditionExample.cpp)
target_link_libraries(ConditionExample PUBLIC
	State
	StateSpace
	Condition
)
target_include_directories(ConditionExample PUBLIC
	${TP_LIBRARY_DIR}
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
	${TP_LIBRARY_DIR}
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
	${TP_LIBRARY_DIR}
	${YAML_INCLUDE_DIRS}
)
