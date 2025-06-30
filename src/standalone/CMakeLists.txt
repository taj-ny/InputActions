pkg_search_module(LIBINPUT REQUIRED libinput)
pkg_search_module(LIBUDEV REQUIRED libudev)
pkg_search_module(WAYLAND_CLIENT REQUIRED wayland-client)

find_package(XKB)

set(inputactions_SOURCES
    input/LibinputInputBackend.cpp
    interfaces/StandaloneInputEmitter.cpp
    interfaces/StandaloneWindowProvider.cpp
    protocols/VirtualKeyboardUnstableV1.cpp
    protocols/WaylandProtocol.cpp
    protocols/WaylandProtocolManager.cpp
    protocols/WlrForeignToplevelManagementV1.cpp
    protocols/WlrVirtualPointerUnstableV1.cpp
    protocols/WlSeat.cpp
    main.cpp
)

add_executable(inputactions_standalone ${inputactions_SOURCES})
target_compile_options(inputactions_standalone PUBLIC -fexceptions)
target_link_libraries(inputactions_standalone PRIVATE
    libinputactions
    ${LIBINPUT_LIBRARIES}
    ${LIBUDEV_LIBRARIES}
    ${WAYLAND_CLIENT_LIBRARIES}
    XKB::XKB
)

function(wayland_protocol name)
    add_custom_command(
        OUTPUT ${CMAKE_SOURCE_DIR}/protocols/${name}.h
        COMMAND wayland-scanner client-header ./protocols/xml/${name}.xml ${CMAKE_SOURCE_DIR}/protocols/${name}.h
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    add_custom_command(
        OUTPUT ${CMAKE_SOURCE_DIR}/protocols/${name}.c
        COMMAND wayland-scanner private-code ./protocols/xml/${name}.xml ${CMAKE_SOURCE_DIR}/protocols/${name}.c
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    target_sources(inputactions_standalone PRIVATE ${CMAKE_SOURCE_DIR}/protocols/${name}.c ${CMAKE_SOURCE_DIR}/protocols/${name}.h)
endfunction()
target_include_directories(inputactions_standalone PRIVATE ${CMAKE_SOURCE_DIR}/protocols)

wayland_protocol(virtual-keyboard-unstable-v1)
wayland_protocol(wlr-foreign-toplevel-management-unstable-v1)
wayland_protocol(wlr-virtual-pointer-unstable-v1)

install(TARGETS inputactions_standalone)