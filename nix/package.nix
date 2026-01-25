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
  version = "0.9.0";

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

  PKG_CONFIG_SYSTEMD_SYSTEMDSYSTEMUNITDIR = "${placeholder "out"}/lib/systemd/system";

  meta = with lib; {
    description = "Linux utility for binding keyboard/mouse/touchpad actions to system actions (standalone implementation)";
    license = licenses.gpl3;
    homepage = "https://github.com/taj-ny/InputActions";
  };
}
