{
  lib,
  cmake,
  extra-cmake-modules,
  hyprland,
  hyprlandPlugins,
  kdePackages,
  libevdev,
  pkg-config,
  yaml-cpp,
  ...
}:

hyprlandPlugins.mkHyprlandPlugin hyprland rec {
  pluginName = "inputactions-hyprland";
  version = "0.7.0";

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
    "-DBUILD_KWIN_EFFECT=OFF"
    "-DBUILD_HYPRLAND_PLUGIN=ON"
  ];

  meta = with lib; {
    description = "Custom touchpad gestures for Plasma 6";
    license = licenses.gpl3;
    homepage = "https://github.com/taj-ny/InputActions";
  };
}
