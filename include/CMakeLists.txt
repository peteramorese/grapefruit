#add_library(EdgeClass edge.cpp)
add_library(GraphClass graph.cpp)
add_library(AstarClass astar.cpp)
add_library(StateSpaceClass stateSpace.cpp)
add_library(StateClass state.cpp)
add_library(ConditionClass condition.cpp)
add_library(TransitionSystemClass transitionSystem.cpp)
#add_library(AutomatonClass automaton.cpp)
target_link_libraries(AstarClass GraphClass)
target_link_libraries(StateClass StateSpaceClass)
#target_link_libraries(StateSpaceClass StateClass)
target_link_libraries(ConditionClass StateClass)
target_link_libraries(TransitionSystemClass GraphClass StateClass ConditionClass AstarClass)
#target_link_libraries(AutomatonClass GraphClass)
add_library(LexSetClass lexSet.cpp)
add_library(SymbSearchClass symbSearch.cpp)
#get_directory_property(GraphClass CMAKE_SOURCE_DIR/include GraphClass)
get_directory_property(TransitionSystemClass include TransitionSystemClass)
target_link_libraries(SymbSearchClass 
	GraphClass 
	LexSetClass
	TransitionSystemClass
	)
