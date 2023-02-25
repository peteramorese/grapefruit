add_executable(DijkstraExample DijkstraExample.cpp)
target_link_libraries(DijkstraExample PUBLIC
)
target_include_directories(DijkstraExample PUBLIC
	${TP_LIBRARY_DIR}
)

add_executable(AStarExample AStarExample.cpp)
target_link_libraries(AStarExample PUBLIC
)
target_include_directories(AStarExample PUBLIC
	${TP_LIBRARY_DIR}
)

add_executable(BODijkstraExample BODijkstraExample.cpp)
target_link_libraries(BODijkstraExample PUBLIC
)
target_include_directories(BODijkstraExample PUBLIC
	${TP_LIBRARY_DIR}
)

add_executable(BOAStarExample BOAStarExample.cpp)
target_link_libraries(BOAStarExample PUBLIC
)
target_include_directories(BOAStarExample PUBLIC
	${TP_LIBRARY_DIR}
)
