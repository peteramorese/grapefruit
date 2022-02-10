#set(pref_include_dir
add_library(LexSetClass lexSet.cpp)
add_library(SymbSearchClass symbSearch.cpp)
#get_directory_property(GraphClass ${CMAKE_SOURCE_DIR}/include GraphClass)
#get_directory_property(TransitionSystemClass include TransitionSystemClass)
target_include_directories(SymbSearchClass PUBLIC ${base_include_dir})
target_link_libraries(SymbSearchClass 
	GraphClass 
	LexSetClass
	TransitionSystemClass
	)
#target_link_libraries(GraphClass LexSetClass)
