add_library(Planners 
    DeterministicTaskPlanner.cpp
)
target_link_libraries(Planners PUBLIC
    Core
)
target_include_directories(Planners PUBLIC
    ${GF_LIBRARY_DIR}
)
