find_package(Threads REQUIRED)

set(SOURCES
	util.c
	store/base.c
	store/memory.c
	store/pair.c
	store/fallback.c
	store/decorator.c
	store/logger.c
	store/array.c
	store/mutex.c
	store/rwlock.c
	resource/base.c
	resource/heap.c
	resource/format.c
	resource/file.c
	resource/pair.c
	resource/first.c
	resource/fallback.c
)

add_library(csalt SHARED
	${SOURCES}
)

add_library(csaltstatic STATIC
	${SOURCES}
)

target_link_libraries(csalt PUBLIC Threads::Threads)
target_link_libraries(csaltstatic PUBLIC Threads::Threads)

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
