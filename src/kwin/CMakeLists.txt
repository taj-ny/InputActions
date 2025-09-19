add_subdirectory(kcm)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Widgets
)

find_package(KWin REQUIRED COMPONENTS
    kwineffects
)
if(${KWin_VERSION} VERSION_LESS 6.2.90)
    message(FATAL_ERROR "InputActions requires Plasma 6.3 or later (installed: ${KWin_VERSION}). See https://wiki.inputactions.org/main/getting-started/installation/kwin for more information.")
endif()
if(${KWin_VERSION} VERSION_GREATER_EQUAL 6.4.80)
    add_compile_definitions(KWIN_6_5_OR_GREATER)
endif()

find_package(XKB REQUIRED)

set(kwin_gestures_SOURCES
    input/KWinInputBackend.cpp
    interfaces/KWinInputEmitter.cpp
    interfaces/KWinOnScreenMessageManager.cpp
    interfaces/KWinPointer.cpp
    interfaces/KWinSessionLock.cpp
    interfaces/KWinWindow.cpp
    interfaces/KWinWindowProvider.cpp
    Effect.cpp
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
