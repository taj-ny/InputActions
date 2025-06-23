# InputActions
Input handler that executes user-defined actions, currently in a very early stage of development.

Supported environments: Hyprland, Plasma 6 Wayland

# Features
- Stroke gesture: draw any shape
  - Multiple strokes can be specified for each gesture
- Mouse gestures: press, stroke, swipe, wheel
  - Horizontal scrolling wheels are supported
  - Multiple mouse buttons can be specified (and all of them must be pressed in any order)
  - Mouse buttons can still be used for normal clicks and dragging, depending on gestures
  - Supports left, middle, right and 24 extra mouse buttons
- Touchpad gestures: click, pinch, press, rotate, stroke, swipe
  - Supports 2-finger swipe and stroke gestures (requires edge scrolling to be disabled)
  - Click gestures (hard press) are only available on touchpads that do not have separate physical buttons below and instead act as one button
  - Absolute positions and pressures of each finger are provided in variables and can be used in conditions - swipe from edge and tip tap gestures are possible
    (see examples)
- Actions: run command, emit input, invoke Plasma global shortcut
  - Executed at a specific point of the gesture's lifecycle (begin, update, end, cancel)
  - Update actions can repeat at a specified interval 
    - Based on time for press gestures and distance for all other ones
    - Bidirectional motion gestures can have actions with negative intervals
- Thresholds: actions and/or gestures will not trigger unless it is reached (based on time/distance just like intervals)
- Compatible with tools that operate at evdev level (Input Actions operates at compositor level, so it will process events after those tools)
- Selective input event blocking: prevents the compositor and clients from receiving events if a gesture is active
- Powerful condition system with [many variables](https://github.com/InputActions/docs/blob/HEAD/variables.md) and [operators](https://github.com/InputActions/docs/blob/HEAD/configuration.md#operators)
  - End conditions: determine whether a gesture is ended or cancelled
- [And more](https://github.com/InputActions/docs/blob/HEAD/configuration.md)

# Installation
[Hyprland](src/hyprland/README.md)<br>
[Plasma](src/kwin/README.md)

[Documentation](https://github.com/InputActions/docs/blob/HEAD/index.md)

# Additional setup (optional)
To gain access to extra touchpad features, create a file at ``/etc/udev/rules.d/71-touchpad.rules``
with the following content:
```
ENV{ID_INPUT_TOUCHPAD}=="1", TAG+="uaccess"
```

This will give all programs read and write access to all touchpads.

# Credits
- [Strokognition](https://invent.kde.org/jpetso/strokognition), [wstroke](https://github.com/dkondor/wstroke), [easystroke](https://github.com/thjaeger/easystroke) - Stroke gestures
- [KWin](https://invent.kde.org/plasma/kwin) - Gesture handling code (heavily extended and modified)
