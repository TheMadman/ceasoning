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
	keystores/keystores.c
)

target_link_libraries(csalt PUBLIC Threads::Threads)

target_include_directories(csalt
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/platforms/posix)

install(TARGETS csalt DESTINATION lib)
install(DIRECTORY csalt DESTINATION include)
install(DIRECTORY platforms/posix/csalt DESTINATION include)
