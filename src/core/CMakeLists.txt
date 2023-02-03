add_library(StateSpace StateSpace.cpp)
target_link_libraries(StateSpace PUBLIC
    yaml-cpp
)
target_include_directories(StateSpace PUBLIC
    ${TP_INCLUDE_DIRS}
)

add_library(State State.cpp)
target_link_libraries(State PUBLIC
    StateSpace
)
target_include_directories(State PUBLIC
    ${TP_INCLUDE_DIRS}
)

add_library(Condition Condition.cpp)
target_link_libraries(Condition PUBLIC
    State
    StateSpace
)
target_include_directories(Condition PUBLIC
    ${TP_INCLUDE_DIRS}
)

add_library(TransitionSystem TransitionSystem.cpp)
target_link_libraries(TransitionSystem PUBLIC
    State
    StateSpace
    Condition
)
target_include_directories(TransitionSystem PUBLIC
    ${TP_INCLUDE_DIRS}
)
