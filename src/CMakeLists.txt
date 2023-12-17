find_package(Threads REQUIRED)

add_library(csalt STATIC
	util.c
	stores/base.c
	stores/memory.c
	stores/pair.c
	stores/fallback.c
	stores/decorator.c
	stores/logger.c
	resources/base.c
	resources/heap.c
	resources/format.c
)

target_link_libraries(csalt PUBLIC Threads::Threads)

target_include_directories(csalt
	PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/platforms/posix>
	$<INSTALL_INTERFACE:include>)

install(DIRECTORY csalt DESTINATION include)
install(DIRECTORY platforms/posix/csalt DESTINATION include)
