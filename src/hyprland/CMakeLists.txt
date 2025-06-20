pkg_check_modules(deps REQUIRED IMPORTED_TARGET
    hyprland
    libdrm
    libinput
    libudev
    pangocairo
    pixman-1
    wayland-server
    xkbcommon
)

set(inputactions_SOURCES
    input/HyprlandInputBackend.cpp
    interfaces/HyprlandInputEmitter.cpp
    main.cpp
    Plugin.cpp
)

add_library(inputactions_hyprland SHARED ${inputactions_SOURCES})
target_compile_definitions(inputactions_hyprland PRIVATE QT_NO_EMIT)
target_compile_options(inputactions_hyprland PUBLIC -fexceptions)
target_link_libraries(inputactions_hyprland PRIVATE
    libinputactions
    rt
    PkgConfig::deps
)

install(TARGETS inputactions_hyprland DESTINATION "lib")