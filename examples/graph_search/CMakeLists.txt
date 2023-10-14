add_executable(DijkstraExample DijkstraExample.cpp)
target_link_libraries(DijkstraExample PUBLIC
)
target_include_directories(DijkstraExample PUBLIC
	${GF_LIBRARY_DIR}
)

add_executable(AStarExample AStarExample.cpp)
target_link_libraries(AStarExample PUBLIC
)
target_include_directories(AStarExample PUBLIC
	${GF_LIBRARY_DIR}
)

add_executable(BODijkstraExample BODijkstraExample.cpp)
target_link_libraries(BODijkstraExample PUBLIC
)
target_include_directories(BODijkstraExample PUBLIC
	${GF_LIBRARY_DIR}
)

add_executable(BOAStarExample BOAStarExample.cpp)
target_link_libraries(BOAStarExample PUBLIC
)
target_include_directories(BOAStarExample PUBLIC
	${GF_LIBRARY_DIR}
)

add_executable(MODijkstraExample MODijkstraExample.cpp)
target_link_libraries(MODijkstraExample PUBLIC
)
target_include_directories(MODijkstraExample PUBLIC
	${GF_LIBRARY_DIR}
)
