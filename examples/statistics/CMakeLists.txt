add_executable(GaussianUpdaterExample GaussianUpdaterExample.cpp)
target_link_libraries(GaussianUpdaterExample PUBLIC
	yaml-cpp
)
target_include_directories(GaussianUpdaterExample PUBLIC
	${TP_LIBRARY_DIR}
	${TP_DEPENDENCY_INCLUDE_DIRS}
)
