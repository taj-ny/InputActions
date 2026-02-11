# InputActions <a href="https://github.com/sponsors/taj-ny"><img src="https://img.shields.io/badge/Sponsor-gray?logo=githubsponsors"></a>
Linux utility for binding keyboard shortcuts, mouse gestures, touchpad gestures and touchscreen gestures to system actions, created with flexibility in mind.

Supported environments: Plasma 6, Hyprland, GNOME, other Wayland compositors (availability of extra features depends on implemented protocols), X11 (base features only)

[Installation & getting started](https://wiki.inputactions.org/main/getting-started) | [Wiki repository](https://github.com/InputActions/wiki)

# Features
- Supported device types: keyboard, mouse, touchpad, touchscreen
- Input event filtering
- Uses libinput, which handles device quirks
- Complementary evdev input backend for better touchpad support (may be disabled in case of issues)
- Conditional triggers
- Built-in action for simulating keyboard and mouse input

<details>
  <summary>Example configuration</summary>

  ```yaml
  device_rules:
    # ignore a device
    - conditions: $name contains YubiKey
      ignore: true

  keyboard:
    gestures:
      # shift + meta + q -> kill window under pointer
      - type: shortcut
        shortcut: [ leftshift, leftmeta, q ]

        actions:
          - on: begin
            command: kill -9 $window_under_pointer_pid

  mouse:
    gestures:
      # right + draw circle clockwise -> open dolphin
      - type: stroke
        strokes: [ 'Gw4A/DELBwxLFRAZWiUXJWM6HzBkSyRKWlQpYShYOasISUXHACVR6Q8WWP0zEmQA' ]
        mouse_buttons: [ right ]

        actions:
          - command: dolphin

      # trigger group - condition is applied to all subtriggers specified in 'gestures'
      - conditions: $window_class == firefox
        gestures:
          # meta + vertical wheel -> volume control
          - type: wheel
            direction: up_down

            conditions: $keyboard_modifiers == meta

            actions:
              - on: update
                interval: '+'
                input:
                  - keyboard: [ volumedown ]

              - on: update
                interval: '-'
                input:
                  - keyboard: [ volumeup ]

      # this trigger will override the one below due to higher priority, but only if firefox is focused
      - type: press
        mouse_buttons: [ middle ]
        instant: true

        conditions: $window_class == firefox

        actions:
          - on: end
            # ...

      - type: press
        mouse_buttons: [ middle ]
        instant: true

        actions:
          - on: end
            # ...

  touchpad:
    gestures:
      # place 2 fingers, at least 1 on the top/bottom edge, then move in circular motion -> circular scrolling
      - type: circle
        fingers: 2
        direction: any

        conditions:
          any:
            - $finger_1_initial_position_percentage_y <= 0.05
            - $finger_2_initial_position_percentage_y <= 0.05
            - $finger_1_initial_position_percentage_y >= 0.95
            - $finger_2_initial_position_percentage_y >= 0.95

        actions:
          - on: update
            interval: -0.5
            input:
              - mouse: [ wheel 0 -1 ]

          - on: update
            interval: 0.5
            input:
              - mouse: [ wheel 0 1 ]

      # place 2 fingers on the left half, then click -> naviate back
      - type: click
        fingers: 2

        conditions:
          - $finger_1_position_percentage_x <= 0.5
          - $finger_2_position_percentage_x <= 0.5

        actions:
          - on: begin
            input:
              - mouse: [ back ]

      # move 3 fingers -> drag window
      - type: swipe
        fingers: 3
        direction: any
        resume_timeout: 500 # optional: allow lifting fingers for 500 ms

        actions:
          - on: begin
            input:
              - keyboard: [ +leftmeta ]
              - mouse: [ +left ]

          - on: update
            input:
              - mouse: [ move_by_delta ]

          - on: end_cancel
            input:
              - keyboard: [ -leftmeta ]
              - mouse: [ -left ]
  ```
</details>

# Acknowledgements
- [Strokognition](https://invent.kde.org/jpetso/strokognition), [wstroke](https://github.com/dkondor/wstroke), [easystroke](https://github.com/thjaeger/easystroke) - Stroke triggers
- [libinput](https://gitlab.freedesktop.org/libinput/libinput) - Input handling, touchscreen trigger recognition
- [circular-scrolling-improved](https://github.com/galundin/circular-scrolling-improved) - Circle triggers
- [KWin](https://invent.kde.org/plasma/kwin) - Trigger handling code (heavily extended and modified)
