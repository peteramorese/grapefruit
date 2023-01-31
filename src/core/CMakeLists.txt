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
