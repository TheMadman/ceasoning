find_package(Threads REQUIRED)

set(SOURCES
	util.c
	stores/base.c
	stores/memory.c
	stores/pair.c
	stores/fallback.c
	stores/decorator.c
	stores/logger.c
	stores/array.c
	stores/mutex.c
	stores/rwlock.c
	resources/base.c
	resources/heap.c
	resources/format.c
)

add_library(csalt SHARED
	${SOURCES}
)

add_library(csaltstatic STATIC
	${SOURCES}
)

target_link_libraries(csalt PUBLIC Threads::Threads)

target_include_directories(csalt
	PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/platforms/posix>
	$<INSTALL_INTERFACE:include>)
target_include_directories(csaltstatic
	PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/platforms/posix>
	$<INSTALL_INTERFACE:include>)

install(DIRECTORY csalt DESTINATION include)
install(DIRECTORY platforms/posix/csalt DESTINATION include)
