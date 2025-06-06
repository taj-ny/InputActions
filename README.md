# Input Actions [![AUR Version](https://img.shields.io/aur/version/input-actions)](https://aur.archlinux.org/packages/input-actions)
Input handler built on top of libinput and KWin, currently in a very early stage of development.

Supported environments: Plasma 6 Wayland

# Features
- Stroke gesture: draw any shape
  - Can specify multiple strokes per gesture
- Mouse gestures: press, stroke, swipe, wheel
  - Horizontal scrolling wheels are supported
  - Multiple mouse buttons can be specified (and all of them must be pressed in any order)
  - Mouse buttons can still be used for normal clicks and dragging, depending on gestures
  - Supports left, middle, right and 24 extra mouse buttons
- Touchpad gestures: pinch, press, rotate, stroke, swipe
  - Supports 2-finger swipe and stroke gestures (requires edge scrolling to be disabled)
- Actions: run command, emit input, invoke Plasma global shortcut
  - Executed at a specific point of the gesture's lifecycle (begin, update, end, cancel)
  - Update actions can repeat at a specified interval 
    - Based on time for press gestures and distance for all other ones
    - Bidirectional motion gestures can have actions with negative intervals
- Thresholds: actions and/or gestures will not trigger unless it is reached (based on time/distance just like intervals)
- Compatible with tools that operate at evdev level (Input Actions operates at compositor level, so it will process events after those tools)
- Selective input event blocking
  - Blocks built-in Plasma gestures if a custom one is activated 
- Powerful condition system with [many variables](https://github.com/InputActions/docs/blob/HEAD/variables.md) and [operators](https://github.com/InputActions/docs/blob/HEAD/configuration.md#operators)
  - End conditions: determine whether a gesture is ended or cancelled
- [And more](https://github.com/InputActions/docs/blob/HEAD/configuration.md)

# Installation
<details>
  <summary>NixOS (flakes)</summary>
  <br>

``flake.nix``:
  ```nix
  {
    inputs = {
      nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";

      inputactions = {
        url = "github:taj-ny/InputActions";
        inputs.nixpkgs.follows = "nixpkgs";
      };
    };
  }
  ```

  ```nix
  { inputs, pkgs, ... }:

  {
    environment.systemPackages = [
      inputs.inputactions.packages.${pkgs.system}.inputactions-kwin
    ];
  }
  ```
</details>

# Building from source
### Dependencies
- CMake
- Extra CMake Modules
- Plasma 6
- Qt 6
- KF6
- KWin development packages

<details>
  <summary>Arch Linux</summary>
  <br>

  ```
  sudo pacman -S --needed base-devel git extra-cmake-modules qt6-tools kwin yaml-cpp
  ```
</details>

<details>
  <summary>Debian-based (KDE Neon, Kubuntu, Ubuntu)</summary>
  <br>

  ```
  sudo apt install git cmake g++ extra-cmake-modules qt6-tools-dev kwin-wayland kwin-dev libkf6configwidgets-dev gettext libkf6kcmutils-dev libyaml-cpp-dev libxkbcommon-dev
  ```
</details>

<details>
  <summary>Fedora 40, 41</summary>
  <br>

  ```
  sudo dnf install git cmake extra-cmake-modules gcc-g++ qt6-qtbase-devel kwin-devel kf6-ki18n-devel kf6-kguiaddons-devel kf6-kcmutils-devel kf6-kconfigwidgets-devel qt6-qtbase kf6-kguiaddons kf6-ki18n wayland-devel yaml-cpp yaml-cpp-devel libepoxy-devel
  ```
</details>

<details>
  <summary>openSUSE</summary>
  <br>

  ```
  sudo zypper in git cmake-full gcc-c++ kf6-extra-cmake-modules kf6-kguiaddons-devel kf6-kconfigwidgets-devel kf6-ki18n-devel kf6-kcmutils-devel "cmake(KF6I18n)" "cmake(KF6KCMUtils)" "cmake(KF6WindowSystem)" "cmake(Qt6Core)" "cmake(Qt6DBus)" "cmake(Qt6Quick)" "cmake(Qt6Widgets)" libepoxy-devel kwin6-devel yaml-cpp-devel libxkbcommon-devel
  ```
</details>

### Building
```sh
git clone https://github.com/taj-ny/InputActions
cd InputActions
mkdir build
cd build
cmake ../ -DCMAKE_INSTALL_PREFIX=/usr
make
sudo make install
```

Remove the *build* directory when rebuilding the effect.

# Usage
> [!NOTE]
> If the plugin stops working after a system update, you will need to rebuild it. Choose cleanBuild when rebuilding the AUR package.

1. Install the plugin.
2. Open the *Desktop Effects* page in *System Settings*.
3. Enable the *Input Actions* effect in the *Tools* category.

[Documentation](https://github.com/InputActions/docs/blob/HEAD/index.md)

# Credits
- [Strokognition](https://invent.kde.org/jpetso/strokognition), [wstroke](https://github.com/dkondor/wstroke), [easystroke](https://github.com/thjaeger/easystroke) - Stroke gestures
- [KWin](https://invent.kde.org/plasma/kwin) - Gesture handling code (heavily extended and modified)
