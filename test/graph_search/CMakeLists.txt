add_executable(TestMOSearchAlgorithms TestMOSearchAlgorithms.cpp)
target_link_libraries(TestMOSearchAlgorithms PUBLIC
)
target_include_directories(TestMOSearchAlgorithms PUBLIC
	${GF_LIBRARY_DIR}
	${GF_DEPENDENCY_INCLUDE_DIRS}
)