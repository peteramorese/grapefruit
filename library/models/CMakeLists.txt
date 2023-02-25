add_library(Manipulator Manipulator.cpp)
target_link_libraries(Manipulator PUBLIC
    TransitionSystem
)
target_include_directories(Manipulator PUBLIC
    ${TP_LIBRARY_DIR}
)

add_library(GridWorldAgent GridWorldAgent.cpp)
target_link_libraries(GridWorldAgent PUBLIC
    TransitionSystem
)
target_include_directories(GridWorldAgent PUBLIC
    ${TP_LIBRARY_DIR}
)
