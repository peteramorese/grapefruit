cmake_minimum_required(VERSION 3.10.2)

project(task_planner)

#add_executable(${PROJECT_NAME} src/test1.cpp)
add_executable(${PROJECT_NAME} src/tp_test.cpp)
add_subdirectory(include)
target_include_directories(${PROJECT_NAME} PUBLIC include)
link_directories(${PROJECT_NAME} PUBLIC include)
#target_link_libraries(${PROJECT_NAME} EdgeClass AstarClass StateClass TransitionSystemClass)
target_link_libraries(${PROJECT_NAME} 
	EdgeClass 
	AstarClass 
	StateClass 
	ConditionClass
        TransitionSystemClass
	)	

