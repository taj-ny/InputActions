set(INPUTACTIONS_DATA_DIR "${CMAKE_INSTALL_PREFIX}/share/inputactions")

pkg_search_module(WAYLAND_CLIENT REQUIRED wayland-client)

add_executable(inputactions-client
    gnome/GNOMEClient.cpp
    plasma/PlasmaClient.cpp
    wayland/WaylandClient.cpp
    wayland/WaylandProtocol.cpp
    wayland/WaylandProtocolManager.cpp
    wayland/WlrForeignToplevelManagementV1.cpp
    Client.cpp
    ClientDBusInterface.cpp
    ClientMessageHandler.cpp
    main.cpp
)

target_compile_options(inputactions-client PUBLIC -fexceptions)
target_link_libraries(inputactions-client PRIVATE
    libinputactions
    ${WAYLAND_CLIENT_LIBRARIES}
)
target_include_directories(inputactions-client PRIVATE ..)
target_compile_definitions(inputactions-client PRIVATE INPUTACTIONS_DATA_DIR="${INPUTACTIONS_DATA_DIR}")

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
    target_sources(inputactions-client PRIVATE ${PROTOCOL_OUTPUT_DIR}/${name}.c ${PROTOCOL_OUTPUT_DIR}/${name}.h)
endfunction()
target_include_directories(inputactions-client PRIVATE ${PROTOCOL_OUTPUT_DIR})

wayland_protocol(wlr-foreign-toplevel-management-unstable-v1)

install(TARGETS inputactions-client)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/gnome/inputactions@inputactions.org DESTINATION "${INPUTACTIONS_DATA_DIR}/gnome")
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/plasma/script.js DESTINATION "${INPUTACTIONS_DATA_DIR}/plasma")