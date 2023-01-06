cmake_minimum_required(VERSION 3.21)

project(Ceasoning VERSION 0.1)

if(BUILD_TESTING)
	# GCC/Clang-specific stuff, don't like
	# maybe replace with CDash?
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 --coverage")
	link_libraries("--coverage")
endif(BUILD_TESTING)

add_subdirectory(src)

## Refactor this out into a separate file?
## Lots of noise for just "install a CMake target"
install(TARGETS csalt
	EXPORT CeasoningTargets
	DESTINATION lib)
install(EXPORT CeasoningTargets
	FILE CeasoningTargets.cmake
	DESTINATION lib/cmake/Ceasoning)
include(CMakePackageConfigHelpers)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
	"${CMAKE_CURRENT_BINARY_DIR}/CeasoningConfig.cmake"
	INSTALL_DESTINATION "lib/cmake/Ceasoning"
	NO_SET_AND_CHECK_MACRO
	NO_CHECK_REQUIRED_COMPONENTS_MACRO)
write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/CeasoningConfigVersion.cmake"
	VERSION "${Ceasoning_VERSION_MAJOR}.${Ceasoning_VERSION_MINOR}"
	COMPATIBILITY AnyNewerVersion)
install(FILES
	"${CMAKE_CURRENT_BINARY_DIR}/CeasoningConfigVersion.cmake"
	"${CMAKE_CURRENT_BINARY_DIR}/CeasoningConfig.cmake"
	DESTINATION lib/cmake/Ceasoning)
export(EXPORT CeasoningTargets
	FILE "${CMAKE_CURRENT_BINARY_DIR}/CeasoningTargets.cmake")

if(BUILD_EXAMPLES)
	add_subdirectory(pages)
endif(BUILD_EXAMPLES)

if(BUILD_TESTING)
	add_custom_target(coverage
		COMMAND lcov --capture --directory "${CMAKE_CURRENT_SOURCE_DIR}" --exclude '${CMAKE_CURRENT_SOURCE_DIR}/tests/**' --exclude '${CMAKE_CURRENT_SOURCE_DIR}/pages/**' -o tracefile.info
		COMMAND genhtml -o htmlcov tracefile.info
		BYPRODUCTS tracefile.info htmlcov
	)
	add_custom_target(coverage_clean
		COMMAND find "${CMAKE_CURRENT_BINARY_DIR}" -name '*.gcda' -o -name '*.gcno' | xargs rm
	)

	enable_testing()
	add_subdirectory(tests)
endif(BUILD_TESTING)
