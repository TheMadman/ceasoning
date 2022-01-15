cmake_minimum_required(VERSION 3.21)

project(Ceasoning VERSION 0.1)

add_subdirectory(src)

if(BUILD_EXAMPLES)
	add_subdirectory(pages)
endif(BUILD_EXAMPLES)

if(BUILD_TESTING)
	enable_testing()
	add_subdirectory(tests)
endif(BUILD_TESTING)
