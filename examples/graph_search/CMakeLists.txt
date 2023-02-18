add_executable(SO_DijkstraExample SO_DijkstraExample.cpp)
target_link_libraries(SO_DijkstraExample PUBLIC
)
target_include_directories(SO_DijkstraExample PUBLIC
	${TP_INCLUDE_DIRS}
)

add_executable(SO_AStarExample SO_AStarExample.cpp)
target_link_libraries(SO_AStarExample PUBLIC
)
target_include_directories(SO_AStarExample PUBLIC
	${TP_INCLUDE_DIRS}
)

add_executable(MO_BODijkstraExample MO_BODijkstraExample.cpp)
target_link_libraries(MO_BODijkstraExample PUBLIC
)
target_include_directories(MO_BODijkstraExample PUBLIC
	${TP_INCLUDE_DIRS}
)

add_executable(MO_BOAStarExample MO_BOAStarExample.cpp)
target_link_libraries(MO_BOAStarExample PUBLIC
)
target_include_directories(MO_BOAStarExample PUBLIC
	${TP_INCLUDE_DIRS}
)
