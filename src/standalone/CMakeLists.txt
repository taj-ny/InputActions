pkg_search_module(LIBINPUT REQUIRED libinput)
pkg_search_module(LIBUDEV REQUIRED libudev)
pkg_search_module(WAYLAND_CLIENT REQUIRED wayland-client)

set(inputactions_SOURCES
    input/StandaloneInputBackend.cpp
    interfaces/DBusInterfaceCollection.cpp
    interfaces/EvdevInputEmitter.cpp
    protocols/WaylandProtocol.cpp
    protocols/WaylandProtocolManager.cpp
    protocols/WlrForeignToplevelManagementV1.cpp
    main.cpp
    resources.qrc
)

add_executable(inputactionsd ${inputactions_SOURCES})
target_compile_options(inputactionsd PUBLIC -fexceptions)
target_link_libraries(inputactionsd PRIVATE
    libinputactions
    ${LIBINPUT_LIBRARIES}
    ${LIBUDEV_LIBRARIES}
    ${WAYLAND_CLIENT_LIBRARIES}
)
target_include_directories(inputactionsd PUBLIC ${LIBEVDEV_INCLUDE_DIRS})

set(PROTOCOL_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/protocols)
file(MAKE_DIRECTORY ${PROTOCOL_OUTPUT_DIR})
function(wayland_protocol name)
    add_custom_command(
        OUTPUT ${PROTOCOL_OUTPUT_DIR}/${name}.h
        COMMAND wayland-scanner client-header ./protocols/xml/${name}.xml ${PROTOCOL_OUTPUT_DIR}/${name}.h
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    add_custom_command(
        OUTPUT ${PROTOCOL_OUTPUT_DIR}/${name}.c
        COMMAND wayland-scanner private-code ./protocols/xml/${name}.xml ${PROTOCOL_OUTPUT_DIR}/${name}.c
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    target_sources(inputactionsd PRIVATE ${PROTOCOL_OUTPUT_DIR}/${name}.c ${PROTOCOL_OUTPUT_DIR}/${name}.h)
endfunction()
target_include_directories(inputactionsd PRIVATE ${PROTOCOL_OUTPUT_DIR})

wayland_protocol(wlr-foreign-toplevel-management-unstable-v1)

install(TARGETS inputactionsd)