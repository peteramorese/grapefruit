add_executable(BenchmarkEFE BenchmarkEFE.cpp)
target_link_libraries(BenchmarkEFE PUBLIC
	Eigen3::Eigen
	Threads::Threads
	Core
	Models
)
target_include_directories(BenchmarkEFE PUBLIC
	${GF_LIBRARY_DIR}
	${GF_DEPENDENCY_INCLUDE_DIRS}
    ${PRL_LIBRARY_DIR}
)

add_executable(FakePlanGenerator FakePlanGenerator.cpp)
target_link_libraries(FakePlanGenerator PUBLIC
	Eigen3::Eigen
	Threads::Threads
	Core
	Models
)
target_include_directories(FakePlanGenerator PUBLIC
	${GF_LIBRARY_DIR}
	${GF_DEPENDENCY_INCLUDE_DIRS}
    ${PRL_LIBRARY_DIR}
)
