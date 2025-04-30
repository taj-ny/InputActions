# Input Actions
Input handler built on top of libinput and KWin, currently in a very early stage of development.

Supported environments: Plasma 6 Wayland

# Features
- Mouse gestures: press, stroke, swipe, wheel
  - **Requires Plasma 6.3**
  - Activated by pressing mouse button(s), keyboard modifier(s) or both
  - Mouse buttons can still be used for normal clicks and motion (a small delay for button presses is introduced, can be changed)
  - Supports horizontal scrolling wheels
- Touchpad gestures: pinch, press, rotate, stroke, swipe
  - Supports 2-finger motion gestures by treating scroll events as swipe events
- Many different ways of performing gestures, can be combined
  - Begin/end position(s) (mouse only for now): Rectangle(s) where the gesture must begin and/or end
  - Pressed keyboard modifiers
  - Pressed mouse buttons
  - Speed: fast, slow
  - Threshold (min and/or max): Time for press gestures, distance for all others
- Stroke gestures: draw any shape
- Actions: run command, simulate input (low-latency, no external tools or input group required), invoke Plasma global shortcut
  - Executed at a specific point of the gesture's lifecycle (begin, update, end, cancel), allowing for complex gestures like 3-finger window drag or alt+tab without them being hard-coded
  - Update actions can repeat at a specified interval
- Bi-directional motion gestures: change direction during a gesture and start executing different update actions - useful for gestures like volume control
- Conditions: Window class, state
- Blocks input events when necessary

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
> If the effect stops working after a system upgrade, you will need to rebuild it.

1. Install the plugin.
2. Open the Desktop Effects page in System Settings.
3. Enable the *Input Actions* effect in the *Tools* category.

[Documentation](docs/index.md)

# Credits
- [Strokognition](https://invent.kde.org/jpetso/strokognition), [wstroke](https://github.com/dkondor/wstroke), [easystroke](https://github.com/thjaeger/easystroke) - Stroke gestures
- [KWin](https://invent.kde.org/plasma/kwin) - Gesture handling code (heavily extended and modified)
