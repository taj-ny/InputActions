add_subdirectory(kcm)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Widgets
)

find_package(KWin REQUIRED COMPONENTS
    kwineffects
)

find_package(XKB REQUIRED)

if(${KWin_VERSION} VERSION_GREATER_EQUAL 6.1.90)
    add_compile_definitions(KWIN_6_2_OR_GREATER)
endif()
if(${KWin_VERSION} VERSION_GREATER_EQUAL 6.2.90)
    add_compile_definitions(KWIN_6_3_OR_GREATER)
endif()

set(kwin_gestures_SOURCES
    input/KWinInputBackend.cpp
    input/KWinKeyboard.cpp
    interfaces/KWinInputEmitter.cpp
    interfaces/KWinOnScreenMessageManager.cpp
    interfaces/KWinPointer.cpp
    interfaces/KWinSessionLock.cpp
    interfaces/KWinWindow.cpp
    interfaces/KWinWindowProvider.cpp
    DBusInterface.cpp
    Effect.cpp
    globals.h
    main.cpp
    utils.h
)

add_library(kwin_gestures MODULE ${kwin_gestures_SOURCES})
target_link_libraries(kwin_gestures PRIVATE
    KWin::kwin
    libinputactions
    Qt6::Widgets
    XKB::XKB
)
target_compile_options(kwin_gestures PUBLIC -fexceptions)

install(TARGETS kwin_gestures DESTINATION ${KDE_INSTALL_PLUGINDIR}/kwin/effects/plugins)
