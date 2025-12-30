# InputActions <a href="https://github.com/sponsors/taj-ny"><img src="https://img.shields.io/badge/Sponsor-gray?logo=githubsponsors"></a>
InputActions is a tool that lets you bind simple and complex input actions (triggers) to system actions.

Supported environments: GNOME, Hyprland, Plasma 6, other Wayland compositors (availability of extra features depends on implemented protocols), X11 (base features only)

[Installation & getting started](https://wiki.inputactions.org/main/getting-started) | [Example triggers](https://wiki.inputactions.org/main/example-triggers) | [Wiki repository](https://github.com/InputActions/wiki)

# Features
- Keyboard triggers: shortcut
- Mouse:
  - Multiple activation mouse buttons can be specified, the press order can be set to exact (for rocker gestures) or any
  - Activation button(s) can be used normally at the cost of a configurable delay, if no triggers are eligible for activation, there is no blocking and thus no
    delay
  - Keyboard modifiers can be specified instead of or along with mouse buttons
  - Triggers:
    - Circle: continuous circular motion
    - Press: can be either instant or delayed
    - Stroke: draw a shape
    - Swipe: move left, right, up or down
    - Wheel (horizontal wheels are supported as well)
- Touchpad:
  - Absolute initial and current positions of all fingers are available as variables and can be used in conditions, making position-dependent triggers possible
  - Triggers:
    - Circle: 1-4 fingers, can be used for circular scrolling
    - Click: 1-5 fingers, available on devices with one integrated button
    - Hold: 1-4 fingers
    - Pinch: 2-4 fingers
    - Rotate: 2-4 fingers
    - Stroke: 1-4 fingers
    - Swipe: 1-4 fingers
    - Tap: 1-5 fingers
- Pointer (any device capable of moving the pointer) triggers: hover
- Conditions: can be set on individual triggers and actions and determine whether the trigger can be activated or the action can be executed. Triggers also
  support end conditions, which are checked at the end and determine whether the trigger is ended or cancelled. Complex AND, OR and NOT conditions are
  possible.
  <br><br>
  ```yaml
  conditions:
    all:
      - $keyboard_modifiers == [ ctrl, meta ]
      - !$window_class matches firefox
      - any:
        - $window_fullscreen
        - $window_maximized
  ```
  [List of variables](https://wiki.inputactions.org/main/variables.html)
- Bidirectional motion triggers: the direction can be changed during a trigger to run different actions of the same trigger
- Actions can be executed at various points of the trigger's lifecycle (begin, tick, update, end, cancel):<br>
  Example touchpad window switching trigger that uses alt+tab:
  ```yaml
  - type: swipe
    fingers: 4
    direction: left_right
  
    actions:
      - on: begin
        input:
          - keyboard: [ +leftalt, tab ]

      - on: update
        interval: -75
        input:
          - keyboard: [ leftshift+tab ]

      - on: update
        interval: 75
        input:
          - keyboard: [ tab ]
 
      - on: end_cancel
        input:
          - keyboard: [ -leftalt ]
  ```
  Tick actions can be used to create time-based actions in motion-based triggers, for example to automatically move the pointer when a finger reaches
  the edge:
  ```yaml
  - type: swipe
    fingers: 1
    direction: any
    block_events: false # allows the pointer to be moved

    conditions: $finger_1_initial_position_percentage between 0.2,0.2;0.8,0.8 # prevent accidental activations

    actions:
      - on: tick
        conditions: $finger_1_position_percentage_x <= 0.05

        input:
          - mouse: [ move_by -1 0 ]

      - on: tick
        conditions: $finger_1_position_percentage_x >= 0.95

        input:
          - mouse: [ move_by 1 0 ]

      - on: tick
        conditions: $finger_1_position_percentage_y <= 0.05

        input:
          - mouse: [ move_by 0 -1 ]

      - on: tick
        conditions: $finger_1_position_percentage_y >= 0.95

        input:
          - mouse: [ move_by 0 1 ]
  ```
- Built-in action for generating input events
- Input event blocking for all trigger types
- InputActions variables can be passed to commands through environment variables:
  ```yaml
  # minimize window under pointer (Plasma)
  actions:
    - on: end
      command: kdotool windowminimize $window_under_id
  ```

# Credits
- [Strokognition](https://invent.kde.org/jpetso/strokognition), [wstroke](https://github.com/dkondor/wstroke), [easystroke](https://github.com/thjaeger/easystroke) - Stroke triggers
- [circular-scrolling-improved](https://github.com/galundin/circular-scrolling-improved) - Circle triggers
- [KWin](https://invent.kde.org/plasma/kwin) - Gesture handling code (heavily extended and modified)
