{
  cmake,
  extra-cmake-modules,
  hyprlandPlugins,
  kdePackages,
  lib,
  libevdev,
  pkg-config,
  yaml-cpp,
  ...
}:

hyprlandPlugins.mkHyprlandPlugin rec {
  pluginName = "inputactions_hyprland";
  version = "0.8.1";

  src = ./..;

  dontWrapQtApps = true;

  nativeBuildInputs = [
    extra-cmake-modules
  ];

  buildInputs = [
    kdePackages.qtbase
    libevdev
    pkg-config
    yaml-cpp
  ];

  cmakeFlags = [
    "-DINPUTACTIONS_BUILD_HYPRLAND=ON"
  ];

  meta = with lib; {
    description = "Linux utility for binding keyboard/mouse/touchpad actions to system actions (Hyprland implementation)";
    license = licenses.gpl3;
    homepage = "https://github.com/taj-ny/InputActions";
  };
}
