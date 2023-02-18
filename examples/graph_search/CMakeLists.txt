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

add_executable(MO_DijkstraExample MO_DijkstraExample.cpp)
target_link_libraries(MO_DijkstraExample PUBLIC
)
target_include_directories(MO_DijkstraExample PUBLIC
	${TP_INCLUDE_DIRS}
)
