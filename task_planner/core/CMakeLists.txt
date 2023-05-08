add_library(Core 
    Automaton.cpp
    Condition.cpp
    State.cpp
    StateSpace.cpp
    TransitionSystem.cpp
)
target_link_libraries(Core PUBLIC
    yaml-cpp
    spot
    bddx
)
target_include_directories(Core PUBLIC
    ${TP_LIBRARY_DIR}
    ${TP_DEPENDENCY_INCLUDE_DIRS}
)
