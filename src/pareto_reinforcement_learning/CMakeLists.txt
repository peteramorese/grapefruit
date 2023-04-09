add_executable(PRLGridWorldAgent PRLGridWorldAgent.cpp)
target_link_libraries(PRLGridWorldAgent PUBLIC
)
target_include_directories(PRLGridWorldAgent PUBLIC
	${TP_LIBRARY_DIR}
	${TP_DEPENDENCY_INCLUDE_DIRS}
)
