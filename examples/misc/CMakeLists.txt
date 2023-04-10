add_executable(TypeGenericArrayExample TypeGenericArrayExample.cpp)
target_link_libraries(TypeGenericArrayExample PUBLIC
)
target_include_directories(TypeGenericArrayExample PUBLIC
	${TP_LIBRARY_DIR}
)

add_executable(Misc Misc.cpp)
target_link_libraries(Misc PUBLIC
	spot
	#/home/peter/usr/lib/libspot.so
)
target_include_directories(Misc PUBLIC
	${TP_LIBRARY_DIR}
	${TP_DEPENDENCY_INCLUDE_DIRS}
	#/home/peter/usr/include
)
