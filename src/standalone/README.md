# InputActions (standalone)
This version does not support Hyprland and Plasma 6 Wayland - use the plugins instead.

# How stuff works
- Active window info
  - [wlr-foreign-toplevel-management-v1](https://wayland.app/protocols/wlr-foreign-toplevel-management-unstable-v1)
- Emitting keyboard events
  - [virtual-keyboard-unstable-v1](https://wayland.app/protocols/virtual-keyboard-unstable-v1)
  - Fallback: uinput
- Emitting mouse events
  - [wlr-virtual-pointer-unstable-v1](https://wayland.app/protocols/wlr-virtual-pointer-unstable-v1)
  - Fallback: uinput