add_executable(GraphExample GraphExample.cpp)
target_link_libraries(GraphExample PUBLIC
	Core
)
target_include_directories(GraphExample PUBLIC
	${GF_LIBRARY_DIR}
)

add_executable(DAGExample DAGExample.cpp)
target_link_libraries(DAGExample PUBLIC
	Core
)
target_include_directories(DAGExample PUBLIC
	${GF_LIBRARY_DIR}
)

add_executable(AutomatonExample AutomatonExample.cpp)
target_link_libraries(AutomatonExample PUBLIC
	Core
)
target_include_directories(AutomatonExample PUBLIC
	${GF_LIBRARY_DIR}
)

add_executable(YAMLExample YAMLExample.cpp)
target_link_libraries(YAMLExample PUBLIC
	yaml-cpp
)
target_include_directories(YAMLExample PUBLIC
	${GF_LIBRARY_DIR}
	${GF_DEPENDENCY_INCLUDE_DIRS}
)

#add_executable(StateExample StateExample.cpp)
#target_link_libraries(StateExample PUBLIC
#	yaml-cpp
#	Core
#)
#target_include_directories(StateExample PUBLIC
#	${GF_LIBRARY_DIR}
#	${GF_DEPENDENCY_INCLUDE_DIRS}
#)

#add_executable(StateSpaceExample StateSpaceExample.cpp)
#target_link_libraries(StateSpaceExample PUBLIC
#	yaml-cpp
#	Core
#)
#target_include_directories(StateSpaceExample PUBLIC
#	${GF_LIBRARY_DIR}
#	${GF_DEPENDENCY_INCLUDE_DIRS}
#)

add_executable(ConditionExample ConditionExample.cpp)
target_link_libraries(ConditionExample PUBLIC
	Core
)
target_include_directories(ConditionExample PUBLIC
	${GF_LIBRARY_DIR}
	${GF_DEPENDENCY_INCLUDE_DIRS}
)

add_executable(TransitionSystemExample TransitionSystemExample.cpp)
target_link_libraries(TransitionSystemExample PUBLIC
	Core
)
target_include_directories(TransitionSystemExample PUBLIC
	${GF_LIBRARY_DIR}
	${GF_DEPENDENCY_INCLUDE_DIRS}
)

add_executable(SymbolicProductAutomatonExample SymbolicProductAutomatonExample.cpp)
target_link_libraries(SymbolicProductAutomatonExample PUBLIC
	Core
	Models
)
target_include_directories(SymbolicProductAutomatonExample PUBLIC
	${GF_LIBRARY_DIR}
	${GF_DEPENDENCY_INCLUDE_DIRS}
)

add_executable(ProductProjectionExample ProductProjectionExample.cpp)
target_link_libraries(ProductProjectionExample PUBLIC
	Core
	Models
)
target_include_directories(ProductProjectionExample PUBLIC
	${GF_LIBRARY_DIR}
	${GF_DEPENDENCY_INCLUDE_DIRS}
)
