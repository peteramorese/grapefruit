add_executable(GaussianUpdaterExample GaussianUpdaterExample.cpp)
target_link_libraries(GaussianUpdaterExample PUBLIC
)
target_include_directories(GaussianUpdaterExample PUBLIC
	${TP_LIBRARY_DIR}
	${TP_DEPENDENCY_INCLUDE_DIRS}
)
