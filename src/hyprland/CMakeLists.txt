pkg_search_module(hyprland REQUIRED hyprland)
pkg_check_modules(deps REQUIRED IMPORTED_TARGET
    libdrm
    libinput
    libudev
    hyprland
    pangocairo
    pixman-1
    wayland-server
    xkbcommon
)

set(inputactions_SOURCES
    input/HyprlandInputBackend.cpp
    interfaces/HyprlandInputEmitter.cpp
    interfaces/HyprlandOnScreenMessageManager.cpp
    interfaces/HyprlandPointer.cpp
    interfaces/HyprlandSessionLock.cpp
    interfaces/HyprlandWindow.cpp
    interfaces/HyprlandWindowProvider.cpp
    utils/HyprlandFunctionHook.cpp
    main.cpp
    Plugin.cpp
)

list(GET hyprland_INCLUDE_DIRS 0 hyprland_FIRST_INCLUDE_DIR)
add_library(inputactions_hyprland SHARED ${inputactions_SOURCES})
target_compile_definitions(inputactions_hyprland PRIVATE QT_NO_EMIT)
target_compile_options(inputactions_hyprland PUBLIC -fexceptions)
target_include_directories(inputactions_hyprland PRIVATE ${hyprland_FIRST_INCLUDE_DIR}/hyprland/src)
target_link_libraries(inputactions_hyprland PRIVATE
    libinputactions
    rt
    PkgConfig::deps
)

install(TARGETS inputactions_hyprland)
