{
  cmake,
  extra-cmake-modules,
  lib,
  libevdev,
  pkg-config,
  qttools,
  stdenv,
  wrapQtAppsHook,
  yaml-cpp,
  ...
}:

stdenv.mkDerivation rec {
  pname = "inputactions-standalone";
  version = "0.8.1";

  src = ./..;

  nativeBuildInputs = [
    cmake
    extra-cmake-modules
    wrapQtAppsHook
  ];

  buildInputs = [
    qttools
    libevdev
    pkg-config
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
