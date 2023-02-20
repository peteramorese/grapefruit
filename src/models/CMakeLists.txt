add_library(Manipulator Manipulator.cpp)
target_link_libraries(Manipulator PUBLIC
    TransitionSystem
)
target_include_directories(Manipulator PUBLIC
    ${TP_INCLUDE_DIRS}
)
