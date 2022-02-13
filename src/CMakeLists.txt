find_package(Threads REQUIRED)

add_library(csalt STATIC
	stores/stores.c
	stores/decorators.c
	stores/compositestores.c
	resources/resources.c
	resources/file.c
	resources/network.c
	resources/decorators.c
	resources/compositeresources.c
)

target_link_libraries(csalt PUBLIC Threads::Threads)

target_include_directories(csalt
	PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/platforms/posix>
	$<INSTALL_INTERFACE:include>
	$<INSTALL_INTERFACE:include/csalt/platforms/posix>)

install(DIRECTORY csalt DESTINATION include)
install(DIRECTORY platforms/posix/csalt DESTINATION include)
