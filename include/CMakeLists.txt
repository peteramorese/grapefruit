
# Base:
add_library(GraphClass base/graph.cpp)
add_library(StateClass base/state.cpp base/stateSpace.cpp)
add_library(ConditionClass base/condition.cpp)
add_library(TransitionSystemClass base/transitionSystem.cpp)
add_library(GameClass base/game.cpp)
add_library(BenchmarkClass base/benchmark.cpp)
target_link_libraries(TransitionSystemClass 
    GraphClass
    StateClass
    ConditionClass
)
target_link_libraries(GameClass 
    TransitionSystemClass
)

# Preference:
add_library(LexSetClass preference/lexSet.cpp)
add_library(SymbSearchClass preference/symbSearch.cpp)
target_link_libraries(SymbSearchClass LexSetClass)


