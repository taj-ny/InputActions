{
  cli11,
  cmake,
  extra-cmake-modules,
  lib,
  qttools,
  stdenv,
  wrapQtAppsHook,
  ...
}:

stdenv.mkDerivation rec {
  pname = "inputactions-ctl";
  version = "0.9.0";

  src = ./..;

  nativeBuildInputs = [
    cmake
    extra-cmake-modules
    wrapQtAppsHook
  ];

  buildInputs = [
    cli11
    qttools
  ];

  cmakeFlags = [
    "-DINPUTACTIONS_BUILD_CTL=ON"
  ];

  meta = with lib; {
    description = "Control tool for InputActions";
    license = licenses.gpl3;
    homepage = "https://github.com/taj-ny/InputActions";
  };
}
