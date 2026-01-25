{
  cmake,
  extra-cmake-modules,
  kglobalacceld,
  kwin,
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
  pname = "inputactions-kwin";
  version = "0.9.0";

  src = ./..;

  nativeBuildInputs = [
    cmake
    extra-cmake-modules
    wrapQtAppsHook
  ];

  buildInputs = [
    kwin
    qttools
    libevdev
    kglobalacceld
    pkg-config
    yaml-cpp
  ];

  cmakeFlags = [
    "-DINPUTACTIONS_BUILD_KWIN=ON"
  ];

  meta = with lib; {
    description = "Linux utility for binding keyboard/mouse/touchpad actions to system actions (KWin implementation)";
    license = licenses.gpl3;
    homepage = "https://github.com/taj-ny/InputActions";
  };
}
