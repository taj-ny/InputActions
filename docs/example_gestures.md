# Example gestures
## Mouse
<details>
  <summary>Right + Draw circle clockwise - Close window</summary>

  ```yaml
  - type: stroke
    strokes: [ 'OAMAAUkEBRZZEgwkYCARMmA6GUBSViRPQGMrYyNkNLAOVT3DAzhH0AUdUOkYC1j2OQBkAA==' ]
    mouse_buttons: [ right ]

    actions:
      - plasma_shortcut: kwin,Window Close
  ```
</details>
<details>
  <summary>Meta + Wheel - Volume control</summary>
  
  ```yaml
  - type: wheel
    direction: up
    keyboard_modifiers: [ meta ]

    actions:
      - input:
        - keyboard: [ volumeup ]

  - type: wheel
    direction: down
    keyboard_modifiers: [ meta ]

    actions:
      - input:
        - keyboard: [ volumedown ]
  ```
</details>
<details>
  <summary>Meta + Left/Right - Go back/forward</summary>

  ```yaml
  - type: press
    keyboard_modifiers: [ meta ]
    mouse_buttons: [ left ]
    instant: true

    actions:
      - on: begin
        input:
          - mouse: [ back ]

  - type: press
    keyboard_modifiers: [ meta ]
    mouse_buttons: [ right ]
    instant: true

    actions:
      - on: begin
        input:
          - mouse: [ forward ]
  ```
</details>
<details>
  <summary>Meta + Left + Swipe up/down - Home/End</summary>
  
  ```yaml
  - type: swipe
    direction: up
    keyboard_modifiers: [ meta ]
    mouse_buttons: [ left ]

    actions:
      - on: begin
        input:
          - keyboard: [ leftctrl+home ]

  - type: swipe
    direction: down
    keyboard_modifiers: [ meta ]
    mouse_buttons: [ left ]

    actions:
      - on: begin
        input:
          - keyboard: [ leftctrl+end ]
  ```
</details>
<details>
  <summary>Left click top left corner - Open dolphin</summary>

  ```yaml
  - type: press
    mouse_buttons: [ left ]
    start_positions: [ 0%;0% - 0.01%;0.01% ]
    instant: true

    actions:
      - on: begin
        command: dolphin
  ```
</details>
<details>
  <summary>Middle click top edge - Maximize window</summary>

  ```yaml
  - type: press
    mouse_buttons: [ middle ]
    start_positions: [ 0%;0% - 100%;0.01% ]
    instant: true

    actions:
      - on: begin
        plasma_shortcut: kwin,Window Maximize
  ```
</details>

## Touchpad
<details>
  <summary>Swipe 3 - Window drag</summary>

  Swipe gestures have a different acceleration profile. You can change *Device.delta_multiplier* to make the gesture faster or slower.

  ```yaml
  - type: swipe
    direction: any
    fingers: 3

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
<details>
  <summary>Swipe 3 left/right - Go back/forward</summary>
  
  ```yaml
  - type: swipe
    fingers: 3
    direction: left

    actions:
      - on: begin
        input:
          - mouse: [ back ]

  - type: swipe
    fingers: 3
    direction: right

    actions:
      - on: begin
        input:
          - mouse: [ forward ]
  ```
  
</details>
<details>
  <summary>Rotate 2 - Volume control</summary>

  ```yaml
  - type: rotate
    fingers: 2
    direction: any

    actions:
      - on: update
        interval: -10
        input:
          - keyboard: [ volumedown ]

      - on: update
        interval: 10
        input:
          - keyboard: [ volumeup ]
  ```
</details>
<details>
  <summary>Swipe 4 left/right - Switch window</summary>

  Swipe slow - Switch window<br>
  Swipe fast - Open alt+tab switcher 

  ```yaml
  - type: swipe
    direction: left_right
    fingers: 4
    speed: fast

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

  # Quick window switching (left)
  - type: swipe
    direction: left
    fingers: 4
    speed: slow

    actions:
      - on: begin
        input:
          - keyboard: [ leftalt+leftshift+tab ]

  # Quick window switching (right)
  - type: swipe
    direction: right
    fingers: 4
    speed: slow

    actions:
      - on: begin
        input:
          - keyboard: [ leftalt+tab ]
  ```
</details>
<details>
  <summary>Swipe 4 down - Exit fullscreen/Unmaximize/Minimize</summary>
  
  ```yaml
  - type: swipe
    fingers: 4
    direction: down

    actions:
      - on: begin
        one:
          - plasma_shortcut: kwin,Window Fullscreen
            conditions:
              - window_state: [ fullscreen ]

          - plasma_shortcut: kwin,Window Maximize
            conditions:
              - window_state: [ maximized ]

          - plasma_shortcut: kwin,Window Minimize
  ```
</details>
