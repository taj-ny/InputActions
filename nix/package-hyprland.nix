{
  cmake,
  extra-cmake-modules,
  hyprland,
  hyprlandPlugins,
  kdePackages,
  lib,
  libevdev,
  pkg-config,
  yaml-cpp,
  ...
}:

hyprlandPlugins.mkHyprlandPlugin hyprland rec {
  pluginName = "inputactions_hyprland";
  version = "0.8.0";

  src = ./..;

  dontWrapQtApps = true;

  nativeBuildInputs = [
    extra-cmake-modules
  ] ++ hyprland.nativeBuildInputs;

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
    description = "Custom mouse and touchpad gestures for Hyprland";
    license = licenses.gpl3;
    homepage = "https://github.com/taj-ny/InputActions";
  };
}
