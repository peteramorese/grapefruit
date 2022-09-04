
# Base:
add_library(GraphClass base/graph.cpp)
add_library(StateClass base/state.cpp base/stateSpace.cpp)
add_library(ConditionClass base/condition.cpp)
add_library(TransitionSystemClass base/transitionSystem.cpp)
add_library(GameClass base/game.cpp)
target_link_libraries(TransitionSystemClass 
    GraphClass
    StateClass
    ConditionClass
)
target_link_libraries(GameClass 
    GraphClass
    TransitionSystemClass
)

# Tools:
add_library(MockGamePlayClass tools/mockGamePlay.cpp)
add_library(BenchmarkClass tools/benchmark.cpp)
add_library(WriteToFileClass tools/writeToFile.cpp)
target_link_libraries(WriteToFileClass stdc++fs)

# Research:
add_library(LexSetClass research/lexSet.cpp)
add_library(SymbSearchClass research/symbSearch.cpp)
add_library(StratSynthesisClass research/stratSynthesis.cpp)
target_link_libraries(SymbSearchClass LexSetClass)
add_library(OrderedPlannerClass research/orderedPlanner.cpp)
target_link_libraries(OrderedPlannerClass TransitionSystemClass BenchmarkClass)

