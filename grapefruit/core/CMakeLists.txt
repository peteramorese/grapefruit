file(GLOB SOURCES *.cpp)

add_library(Core ${SOURCES})
target_link_libraries(Core PUBLIC
    yaml-cpp
    spot
    bddx
)
target_include_directories(Core PUBLIC
    ${GF_LIBRARY_DIR}
    ${GF_DEPENDENCY_INCLUDE_DIRS}
)
