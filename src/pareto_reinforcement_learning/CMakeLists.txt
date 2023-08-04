add_executable(PRLGridWorldAgent PRLGridWorldAgent.cpp)
target_link_libraries(PRLGridWorldAgent PUBLIC
	Eigen3::Eigen
	Threads::Threads
	Core
	Models
)
target_include_directories(PRLGridWorldAgent PUBLIC
	${GF_LIBRARY_DIR}
	${GF_DEPENDENCY_INCLUDE_DIRS}
)

add_executable(PRLGridWorldAgentBenchmark PRLGridWorldAgentBenchmark.cpp)
target_link_libraries(PRLGridWorldAgentBenchmark PUBLIC
	Eigen3::Eigen
	Threads::Threads
	Core
	Models
)
target_include_directories(PRLGridWorldAgentBenchmark PUBLIC
	${GF_LIBRARY_DIR}
	${GF_DEPENDENCY_INCLUDE_DIRS}
)
