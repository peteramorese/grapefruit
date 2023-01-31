add_library(StateSpace StateSpace.cpp)
target_link_libraries(StateSpace PUBLIC
    yaml-cpp
)
target_include_directories(StateSpace PUBLIC
    ${TP_INCLUDE_DIRS}
)
