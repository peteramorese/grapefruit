add_executable(GaussianUpdaterExample GaussianUpdaterExample.cpp)
target_link_libraries(GaussianUpdaterExample PUBLIC
	yaml-cpp
)
target_include_directories(GaussianUpdaterExample PUBLIC
	${GF_LIBRARY_DIR}
	${GF_DEPENDENCY_INCLUDE_DIRS}
)
