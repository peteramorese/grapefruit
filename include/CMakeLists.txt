
# Base:
add_library(GraphClass base/graph.cpp)
add_library(StateClass base/state.cpp base/stateSpace.cpp)
add_library(ConditionClass base/condition.cpp)
add_library(TransitionSystemClass base/transitionSystem.cpp)
add_library(GameClass base/game.cpp)
add_library(BenchmarkClass base/benchmark.cpp)
add_library(MockGamePlayClass base/mockGamePlay.cpp)
target_link_libraries(TransitionSystemClass 
    GraphClass
    StateClass
    ConditionClass
)
target_link_libraries(GameClass 
    GraphClass
    TransitionSystemClass
)

# Research:
add_library(LexSetClass research/lexSet.cpp)
add_library(SymbSearchClass research/symbSearch.cpp)
add_library(StratSynthesisClass research/stratSynthesis.cpp)
target_link_libraries(SymbSearchClass LexSetClass)
add_library(OrderedPlannerClass research/orderedPlanner.cpp)
target_link_libraries(OrderedPlannerClass TransitionSystemClass)


