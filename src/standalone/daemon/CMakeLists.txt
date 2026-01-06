pkg_search_module(LIBINPUT REQUIRED libinput)
pkg_search_module(LIBUDEV REQUIRED libudev)

add_executable(inputactionsd
    input/StandaloneInputBackend.cpp
    input/StandaloneInputDevice.cpp
    interfaces/EvdevInputEmitter.cpp
    interfaces/IPCEnvironmentInterfaces.cpp
    interfaces/IPCNotificationManager.cpp
    interfaces/IPCPlasmaGlobalShortcutInvoker.cpp
    interfaces/IPCProcessRunner.cpp
    main.cpp
    Server.cpp
    SessionManager.cpp
)
target_compile_options(inputactionsd PUBLIC -fexceptions)
target_link_libraries(inputactionsd PRIVATE
    libinput-cpp
    libinputactions
    ${LIBINPUT_LIBRARIES}
    ${LIBUDEV_LIBRARIES}
    Qt6::Network
)
target_include_directories(inputactionsd PUBLIC
    ${LIBEVDEV_INCLUDE_DIRS}
    ${LIBINPUT_INCLUDE_DIRS}
)

install(TARGETS inputactionsd)

if(INPUTACTIONS_SYSTEMD)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/inputactionsd.service.in ${CMAKE_CURRENT_BINARY_DIR}/inputactionsd.service @ONLY)
    pkg_get_variable(SYSTEMD_SERVICE_DIR systemd systemdsystemunitdir)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/inputactionsd.service DESTINATION ${SYSTEMD_SERVICE_DIR})
endif()