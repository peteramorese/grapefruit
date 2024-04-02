file(GLOB SOURCES *.cpp)

add_library(Tools ${SOURCES})
target_link_libraries(Tools PUBLIC
    yaml-cpp
    #spot
    #bddx
)
target_include_directories(Tools PUBLIC
    ${GF_LIBRARY_DIR}
    ${GF_DEPENDENCY_INCLUDE_DIRS}
)
