add_executable(TypeGenericArrayExample TypeGenericArrayExample.cpp)
target_link_libraries(TypeGenericArrayExample PUBLIC
)
target_include_directories(TypeGenericArrayExample PUBLIC
	${GF_LIBRARY_DIR}
)

add_executable(Misc Misc.cpp)
target_link_libraries(Misc PUBLIC
	Core
	Models
	spot
)
target_include_directories(Misc PUBLIC
	${GF_LIBRARY_DIR}
	${GF_DEPENDENCY_INCLUDE_DIRS}
)

add_executable(GridWorldConfigExample GridWorldConfigExample.cpp)
target_link_libraries(GridWorldConfigExample PUBLIC
	spot
	Models
)
target_include_directories(GridWorldConfigExample PUBLIC
	${GF_LIBRARY_DIR}
	${GF_DEPENDENCY_INCLUDE_DIRS}
)

add_executable(ArgParserExample ArgParserExample.cpp)
target_link_libraries(ArgParserExample PUBLIC
)
target_include_directories(ArgParserExample PUBLIC
	${GF_LIBRARY_DIR}
)
