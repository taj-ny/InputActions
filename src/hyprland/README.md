# InputActions (Hyprland)
InputActions plugin for Hyprland. Build by setting ``INPUTACTIONS_BUILD_HYPRLAND=ON``.

Touchpad hold and pinch gestures and the *cursor_shape* variable are currently only available on x86_64.

# Installation
## Packages
<details>
  <summary>NixOS (flakes, home-manager)</summary>

  ``flake.nix``:
  ```nix
  {
    inputs = {
      inputactions = {
        url = "github:taj-ny/InputActions";
        inputs.nixpkgs.follows = "nixpkgs";
        inputs.hyprland.follows = "hyprland"; # Use if hyprland is in inputs too
      };
    };
  }
  ```

  ```nix
  {
    wayland.windowManager.hyprland.plugins = [
      inputs.inputactions.packages.${pkgs.system}.inputactions-hyprland
    ];
  }
  ```
</details>

## hyprpm
### Dependencies
<details>
  <summary>Arch Linux</summary>

  ```
  sudo pacman -S --needed base-devel git extra-cmake-modules qt6-tools yaml-cpp libevdev
  ```
</details>

### Installation
Read https://wiki.hypr.land/Plugins/Using-Plugins first.
```
hyprpm add https://github.com/taj-ny/InputActions
```