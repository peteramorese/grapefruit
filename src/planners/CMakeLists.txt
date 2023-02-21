add_library(DeterministicTaskPlanner DeterministicTaskPlanner.cpp)
target_link_libraries(DeterministicTaskPlanner PUBLIC
    State
    TransitionSystem
)
target_include_directories(DeterministicTaskPlanner PUBLIC
    ${TP_INCLUDE_DIRS}
)
