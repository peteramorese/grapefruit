add_executable(SO_DijkstraExample SO_DijkstraExample.cpp)
target_link_libraries(SO_DijkstraExample PUBLIC
)
target_include_directories(SO_DijkstraExample PUBLIC
	${TP_INCLUDE_DIRS}
)

#add_executable(SO_AStarExample SO_AStarExample.cpp)
#target_link_libraries(SO_AStarExample PUBLIC
#)
#target_include_directories(SO_AStarExample PUBLIC
#	${TP_INCLUDE_DIRS}
#)
