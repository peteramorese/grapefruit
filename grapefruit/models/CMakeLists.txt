add_library(Models 
    Manipulator.cpp
    GridWorldAgent.cpp
)
target_link_libraries(Models PUBLIC
    Core
)
target_include_directories(Models PUBLIC
    ${GF_LIBRARY_DIR}
)