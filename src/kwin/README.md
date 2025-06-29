# InputActions (KWin)
InputActions plugin for KWin. Build by setting ``INPUTACTIONS_BUILD_KWIN=ON``.

Minimum Plasma version: 6.1, mouse gestures require 6.3

# Installation
## Packages
<details>
  <summary>Arch Linux (AUR)*</summary>

  **Always choose *cleanBuild* when reinstalling the package.**

  https://aur.archlinux.org/packages/inputactions-kwin
  ```
  yay -S inputactions-kwin
  ```
</details>
<details>
  <summary>NixOS (flakes)</summary>

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

**\* Unofficial package, use at your own risk.**

## Manual
### Dependencies
<details>
  <summary>Arch Linux</summary>

  ```
  sudo pacman -S --needed base-devel git extra-cmake-modules qt6-tools kwin yaml-cpp libevdev
  ```
</details>
<details>
  <summary>Debian-based (KDE Neon, Kubuntu, Ubuntu)</summary>

  ```
  sudo apt install git cmake g++ extra-cmake-modules qt6-tools-dev kwin-wayland kwin-dev libkf6configwidgets-dev gettext libkf6kcmutils-dev libyaml-cpp-dev libxkbcommon-dev pkg-config libevdev-dev
  ```
</details>
<details>
  <summary>Fedora 40 - 42</summary>

  ```
  sudo dnf install git cmake extra-cmake-modules gcc-g++ qt6-qtbase-devel kwin-devel kf6-ki18n-devel kf6-kguiaddons-devel kf6-kcmutils-devel kf6-kconfigwidgets-devel qt6-qtbase kf6-kguiaddons kf6-ki18n wayland-devel yaml-cpp yaml-cpp-devel libepoxy-devel libevdev libevdev-devel libdrm-devel
  ```
</details>
<details>
  <summary>openSUSE</summary>

  ```
  sudo zypper in git cmake-full gcc-c++ kf6-extra-cmake-modules kf6-kguiaddons-devel kf6-kconfigwidgets-devel kf6-ki18n-devel kf6-kcmutils-devel "cmake(KF6I18n)" "cmake(KF6KCMUtils)" "cmake(KF6WindowSystem)" "cmake(Qt6Core)" "cmake(Qt6DBus)" "cmake(Qt6Quick)" "cmake(Qt6Widgets)" libepoxy-devel kwin6-devel yaml-cpp-devel libxkbcommon-devel libevdev-devel
  ```
</details>

### Installation
```sh
git clone https://github.com/taj-ny/InputActions
cd InputActions
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DINPUTACTIONS_BUILD_KWIN=ON
make -j$(nproc)
sudo make install
```

> [!NOTE]
> The plugin must be rebuilt every single Plasma update. When rebuilding, remove the *build* directory before and restart Plasma after.

# Usage
1. Open the *Desktop Effects* page in *System Settings*.
2. Enable the *Input Actions* effect in the *Tools* category.
