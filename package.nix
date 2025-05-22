{ lib
, stdenv
, cmake
, extra-cmake-modules
, kwin
, wrapQtAppsHook
, qttools
, kglobalacceld
, yaml-cpp
, libevdev
, pkg-config
, debug ? false
}:

stdenv.mkDerivation rec {
  pname = "inputactions-kwin";
  version = "0.6.0";

  src = ./.;

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

  cmakeFlags = lib.optionals debug [
    "-DCMAKE_BUILD_TYPE=DEBUG"
  ];

  meta = with lib; {
    description = "Custom touchpad gestures for Plasma 6";
    license = licenses.gpl3;
    homepage = "https://github.com/taj-ny/InputActions";
  };
}
