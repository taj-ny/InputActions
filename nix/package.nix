{
  cmake,
  extra-cmake-modules,
  lib,
  libevdev,
  libinput,
  pkg-config,
  qttools,
  stdenv,
  wayland,
  wayland-scanner,
  wrapQtAppsHook,
  yaml-cpp,
  ...
}:

stdenv.mkDerivation rec {
  pname = "inputactions";
  version = "0.8.1";

  src = ./..;

  nativeBuildInputs = [
    cmake
    extra-cmake-modules
    wayland-scanner
    wrapQtAppsHook
  ];

  buildInputs = [
    qttools
    libevdev
    libinput
    pkg-config
    wayland
    yaml-cpp
  ];

  cmakeFlags = [
    "-DINPUTACTIONS_BUILD_STANDALONE=ON"
  ];

  meta = with lib; {
    description = "Custom mouse and touchpad gestures for Plasma 6 Wayland";
    license = licenses.gpl3;
    homepage = "https://github.com/taj-ny/InputActions";
  };
}
