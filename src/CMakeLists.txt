find_package(Threads REQUIRED)

add_library(csalt STATIC
	stores/base.c
	stores/pair.c
	stores/fallback.c
	resources/base.c
)

target_link_libraries(csalt PUBLIC Threads::Threads)

target_include_directories(csalt
	PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/platforms/posix>
	$<INSTALL_INTERFACE:include>)

install(DIRECTORY csalt DESTINATION include)
install(DIRECTORY platforms/posix/csalt DESTINATION include)
