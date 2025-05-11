# Configuration
> [!IMPORTANT]
> Depending on your threat model, it may be desired to restrict write access to the configuration file, as the plugin can block input events, simulate them and run commands as the user, allowing for escaping improperly configured sandboxes.
>
> Until v1.0.0 is released, breaking changes may be introduced at any time, after that only major versions (v2.0.0, v3.0.0 etc.) will introduce breaking changes.

This page only explains the configuration structure, to learn how certain features of the plugin actually work, see [index.md](index.md).

There is currently no configuration UI. The configuration file is located at ``~/.config/kwingestures.yml``. It is created automatically when the plugin is loaded. The plugin should be reconfigured when the file changes. If it doesn't, disable and enable it manually or run ``qdbus org.kde.KWin /Effects org.kde.kwin.Effects.reconfigureEffect kwin_gestures``.

When the configuration fails to load, the error will be logged. To see the last five, run ``journalctl --boot=0 -g "inputactions:" -n 5``. The message should contain the approximate position of where the error is located.
```
inputactions: Failed to load configuration: Invalid swipe direction (line 4, column 17)
```

# Configuration file structure
Bold properties must be set.

## Inheritance
``B : A`` means that B inherits all properties from A unless stated otherwise. 

## Types
| Type                        | Description                                                                                                                                                                                  |
|-----------------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| bool                        | *true* or *false*                                                                                                                                                                            |
| float                       | Positive or negative number with decimal places.                                                                                                                                             |
| int                         | Positive or negative number.                                                                                                                                                                 |
| regex                       | Regular expression                                                                                                                                                                           |
| string                      | Text                                                                                                                                                                                         |
| time                        | Time in milliseconds.                                                                                                                                                                        |
| uint                        | Positive number.                                                                                                                                                                             |
| enum(value1, value2, ...)   | Single value from the list of valid values in brackets.<br><br>Example: ``value2``                                                                                                           |
| flags(value1, value2, ...)  | List of one or multiple values from the list of valid values in brackets.<br><br>Example: ``[ value1, value2 ]``                                                                             |
| list(type)                  | List containing elements of *type*.<br><br>Example: ``list(int)`` - ``[ 1, 2, 3 ]`` or:<br>- 1<br>- 2<br>- 3                                                                                 |
| point                       | Format: ``x,y``                                                                                                                                                                              |
| range(type)                 | Range of numbers of *type*. Format: ``min-max``. ``-`` may be surrounded by exactly one space on each side.<br><br>Example: ``range(int)`` - ``1 - 2``, ``range(point)`` - ``0;0 - 0.5;0.5`` |

## Root
| Property    | Type                                                                                                                     | Description                                                                    | Default |
|-------------|--------------------------------------------------------------------------------------------------------------------------|--------------------------------------------------------------------------------|---------|
| autoreload  | *bool*                                                                                                                   | Whether the configuration should be automatically reloaded on file change.     | *true*  |
| mouse       | *[MouseEventHandler](#mouseeventhandler--eventhandler)* or *list([MouseEventHandler](#mouseeventhandler--eventhandler))* | A list is only necessary if you need different gestures for different devices. |         |
| touchpad    | *[TouchpadEventHandler](#touchpadeventhandler--eventhandler)*                                                            |                                                                                |         |

## EventHandler
| Property     | Type                                                         | Description                                                                              |
|--------------|--------------------------------------------------------------|------------------------------------------------------------------------------------------|
| **gestures** | *list([Gesture](#gesture) or [GestureGroup](#gesturegroup))* |                                                                                          |
| blacklist    | *list(string)*                                               | Names of devices that should be ignored.<br><br>Mutually exclusive with *whitelist*.     |
| speed        | *[Speed](#speed)*                                            | Settings for how gesture speed is determined.                                            |
| whitelist    | *list(string)*                                               | Names of devices that should not be ignored.<br><br>Mutually exclusive with *blacklist*. |

### MouseEventHandler : [EventHandler](#eventhandler)
Supports trackpoints as well.

| Property                   | Type   | Description                                                                                                                                                                                                                      | Default |
|----------------------------|--------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|---------|
| motion_timeout             | *time* | The time during which a motion gesture must be performed. If not, a press gesture will be started. If no press gestures are activated, all pressed mouse buttons will actually be pressed, after having been blocked previously. | *200*   |
| press_timeout              | *time* | The time during which press gestures are not started in case the user presses more than one mouse button.<br><br>Swipe and wheel gesture aren't affected by this option.                                                         | *50*    |
| unblock_buttons_on_timeout | *bool* | Whether blocked mouse buttons should be pressed immediately on timeout. If false, they will be pressed and instantly released on button release.                                                                                 | *true*  |

### TouchpadEventHandler : [EventHandler](#eventhandler)
The *blacklist* and *whitelist* properties are currently not supported for touchpads.

| Property         | Type    | Description                                                                | Default |
|------------------|---------|----------------------------------------------------------------------------|---------|
| delta_multiplier | *float* | Delta multiplier used for *move_by_delta* mouse input actions.             | *1.0*   |
| scroll_timeout   | *time*  | The time of inactivity after which 2-finger motion gestures will be ended. | *100*   |

## Speed
The defaults may not work for everyone, as they depend on the device's sensitivity and size.

| Property            | Type    | Description                                                                                                                                                                                                                                                                                                                                                           | Default |
|---------------------|---------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|---------|
| events              | *uint*  | How many input events to sample in order to determine the speed at which the gesture is performed. The average of each event's delta is compared against the thresholds below. If the threshold is reached, the gesture is considered to have been performed fast, otherwise slow.<br><br>**Note**: No gestures will be triggered until all events have been sampled. | *3*     |
| swipe_threshold     | *float* |                                                                                                                                                                                                                                                                                                                                                                       | *20*    |
| pinch_in_threshold  | *float* |                                                                                                                                                                                                                                                                                                                                                                       | *0.04*  |
| pinch_out_threshold | *float* |                                                                                                                                                                                                                                                                                                                                                                       | *0.08*  |
| rotate_threshold    | *float* |                                                                                                                                                                                                                                                                                                                                                                       | *5*     |

## Gesture
See [example_gestures.md](example_gestures.md) for examples.

| Property               | Type                                                         | Description                                                                                                                                                                     | Default                                                  |
|------------------------|--------------------------------------------------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|----------------------------------------------------------|
| **type**               | *enum(pinch, press, rotate, stroke, swipe, wheel)*           | *hold* is a deprecated alias for *press*.                                                                                                                                       |                                                          |
| actions                | *list([Action](#action))*                                    |                                                                                                                                                                                 |                                                          |
| clear_modifiers        | *bool*                                                       | Whether keyboard modifiers should be cleared when this gesture begins.                                                                                                          | *true* if gesture has an input action, *false* otherwise |
| conditions             | *[Condition](#condition)* or *list([Condition](#condition))* | Must be satisfied in order for this gesture to be activated.<br><br>Condition lists not in a group will be put in an *all* [ConditionGroup](#conditiongroup--condition).        |                                                          |                                                          |
| end_conditions         | *[Condition](#condition)* or *list([Condition](#condition))* | If satisfied, the gesture will end, otherwise it will be cancelled.<br><br>Condition lists not in a group will be put in an *all* [ConditionGroup](#conditiongroup--condition). |                                                          |
| mouse_buttons          | *flags(left, middle, right, back, forward)*                  | Mouse buttons, all of which must be pressed in order for the gesture to be activated.                                                                                           |                                                          |
| name                   | *string*                                                     | Available in debug logs.                                                                                                                                                        |                                                          |
| speed                  | enum(fast, slow)                                             | The speed at which the gesture must be performed. Does not apply to press gestures.<br><br>Will be available as a variable in the future.                                       |                                                          |
| threshold              | *float* (min) or *range(float)* (min and max)                | How far this gesture needs to progress in order to begin.<br><br>Gestures with *begin* or *update* actions can't have maximum thresholds.                                       |                                                          |
| ~~fingers~~            |                                                              | **Deprecated.** Use the *fingers* variable in a [VariableCondition](#variablecondition--condition) instead.                                                                     |                                                          |
| ~~keyboard_modifiers~~ |                                                              | **Deprecated.** Use the *keyboard_modifiers* variable in a [VariableCondition](#variablecondition--condition) instead.                                                          |                                                          |

### RotateGesture : [Gesture](#gesture)
| Property      | Type                                     | Description                                                                         |
|---------------|------------------------------------------|-------------------------------------------------------------------------------------|
| **direction** | *enum(clockwise, counterclockwise, any)* | *any* is a bi-directional gesture. The direction can be changed during the gesture. |

### PinchGesture : [Gesture](#gesture)
| Property      | Type                 | Description                                                                         |
|---------------|----------------------|-------------------------------------------------------------------------------------|
| **direction** | *enum(in, out, any)* | *any* is a bi-directional gesture. The direction can be changed during the gesture. |

### PressGesture : [Gesture](#gesture)
| Property | Type   | Description                                                                                                                                                                                       | Default |
|----------|--------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|---------|
| instant  | *bool* | Whether the gesture should begin immediately. By default, there is a delay to prevent conflicts with normal clicks and stroke/swipe gestures.<br><br>Currently only supported for mouse gestures. | *false* |

### StrokeGesture : [Gesture](#gesture)
| Property    | Type           | Description                                                                                                    |
|-------------|----------------|----------------------------------------------------------------------------------------------------------------|
| **strokes** | *list(string)* | Base64-encoded strings containing the processed strokes. Can be obtained from the stroke recorder in settings. |

### SwipeGesture : [Gesture](#gesture), WheelGesture : [Gesture](#gesture)
| Property      | Type                                                    | Description                                                                                                     |
|---------------|---------------------------------------------------------|-----------------------------------------------------------------------------------------------------------------|
| **direction** | *enum(left, right, up, down, left_right, up_down, any)* | *any*, *left_right* and *up_down* are bi-directional gestures. The direction can be changed during the gesture. |

## Condition
Support for legacy conditions from versions before v0.6 will be removed in v0.7. They should not be mixed with the new ones in a single *conditions/end_conditions* property.

### ConditionGroup : [Condition](#condition)
Contains one or more conditions. Group type is determined by the presence of one of the following properties, all of which are mutually exclusive with each other.

| Property | Type                                      | Description                               |
|----------|-------------------------------------------|-------------------------------------------|
| all      | *list([Condition](#condition))*           | All conditions must be satisfied.         |
| any      | *list([Condition](#condition))*           | At least one condition must be satisfied. |
| none     | *list([Condition](#condition))*           | No conditions must be satisfied.          |

### VariableCondition : [Condition](#condition)
Variable conditions have the following format:
```
[!]$(variable_name) (operator) (value)
```

An exclamation mark placed before ``$`` will negate the condition.

The value can be:
- a value of the same type as *variable_name* (automatically converted to lists when necessary, no need to put single values in ``[]``),
- a list of values of the same types as *variable_name*, may be empty (``[]``).

#### Variables
Variables are currently only used in conditions and cannot be created by users.

| Name                               | Type                                                                                                                                                                                                                                                                                              | Description                                                                                                                                                                                            |
|------------------------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| fingers                            | *number*                                                                                                                                                                                                                                                                                          | Amount of fingers currently on the input device.<br><br>Does not change thorough the gesture.                                                                                                          |
| keyboard_modifiers                 | *flags(alt, ctrl, meta, shift)*                                                                                                                                                                                                                                                                   | Currently pressed keyboard modifiers.<br><br>Does not change thorough the gesture.                                                                                                                     |
| pointer_position_screen_percentage | *point*                                                                                                                                                                                                                                                                                           | Pointer position relative to the top-left corner of the screen it is currently on as a percentage.                                                                                                     |
| pointer_position_window_percentage | *point*                                                                                                                                                                                                                                                                                           | Pointer position relative to the top-left corner of the window it is currently hovering over as a percentage.                                                                                          |
| pointer_shape                      | *enum(alias, all_scroll, col_resize, copy, crosshair, default, e_resize, ew_resize, grab, grabbing, help, move, n_resize, ne_resize, nesw_resize, not_allowed, ns_resize, nw_resize, nwse_resize, pointer, progress, row_resize, s_resize, se_resize, sw_resize, text, up_arrow, w_resize, wait)* | See table at https://developer.mozilla.org/en-US/docs/Web/CSS/cursor#syntax for preview.                                                                                                               |
| window{_under}_class               | *string*                                                                                                                                                                                                                                                                                          | The window's resource class.                                                                                                                                                                           |
| window{_under}_fullscreen          | *bool*                                                                                                                                                                                                                                                                                            | Whether the window is fullscreen.                                                                                                                                                                      |
| window{_under}_maximized           | *bool*                                                                                                                                                                                                                                                                                            | Whether the active window is maximized.                                                                                                                                                                |
| window{_under}_name                | *string*                                                                                                                                                                                                                                                                                          | The window's resource name.                                                                                                                                                                            |
| window{_under}_title               | *string*                                                                                                                                                                                                                                                                                          |                                                                                                                                                                                                        |

All variables of the *point* type have variants with the ``_x`` and ``_y`` suffixes that return the X and Y values respectively.

``window_`` variables have ``window_under_`` variants that return information about the window the pointer is hovering over.


#### Operators
*T2* must be the same as *T1* unless stated otherwise.

| Operator   | Variable type (T1) | Value type (T2) | Description                                                                                                    |
|------------|--------------------|-----------------|----------------------------------------------------------------------------------------------------------------|
| ==         | all                |                 | Equal to.                                                                                                      |
| !=         | all                |                 | Not equal to.                                                                                                  |
| one_of     | all                | *list(T1)*      | Whether the variable's value is present in the specified list.                                                 |
| \>         | *number*, *point*  |                 | Greater than.                                                                                                  |
| \<         | *number*, *point*  |                 | Less than.                                                                                                     |
| \>=        | *number*, *point*  |                 | Greater than or equal to.                                                                                      |
| \<=        | *number*, *point*  |                 | Less than or equal to.                                                                                         |
| between    | *number*, *point*  |                 | Whether the value fits within the specified range (inclusive).<br><br>Format: ``a;b``                          |
| contains   | *flags*, *string*  |                 | Whether the string value contains the specified string or the flags value contains all of the specified flags. |
| matches    | *string*           | *regex*         | Whether the value matches the specified regular expression.                                                    |

#### Example
```yaml
conditions:
  all:
    - any:
        - $fingers between 3;4
        - $keyboard_modifiers == [] # No modifiers must be pressed
    - !$keyboard_modifiers contains meta # Any modifiers are allowed as long as meta isn't one of them
    - !$window_class one_of [ Terraria.bin.x86_64, some, other, blacklisted, games ]
    - $pointer_position_window_percentage_x < .5 # Left side
    - $pointer_position_screen_percentage >= .99 # Bottom-right corner
    - $pointer_position_window_percentage between .4,.4;.6,.6 # Middle
    - $pointer_shape != text
```

### Action
| Property        | Type                                                         | Description                                                                                                                                                                                                                                                                                                                                                                                                                                                          | Default |
|-----------------|--------------------------------------------------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|---------|
| on              | *enum(begin, end, cancel, update, end_cancel)*               | At which point of the gesture's lifecycle the action should be executed.                                                                                                                                                                                                                                                                                                                                                                                             | *end*   |
| conditions      | *[Condition](#condition)* or *list([Condition](#condition))* | Same as *[Gesture](#gesture).conditions*, but only for this action.<br><br>Condition lists not in a group will be put in an *all* [ConditionGroup](#conditiongroup--condition).                                                                                                                                                                                                                                                                                      |         |
| interval        | *float* or *string*                                          | How often an *update* action should execute. Can be negative for bi-directional gestures.<br><br>``0`` - Execute exactly once per event<br>``'+'`` - Execute exactly once per event with positive delta<br>``'-'`` - Execute exactly once per event with negative delta<br> ``number`` - Execute when total delta is positive and is equal to or larger than *number*<br>``-number`` - Execute when total delta is negative and is equal to or smaller than *number* | *0*     |
| name            | *string*                                                     | Available in debug logs.                                                                                                                                                                                                                                                                                                                                                                                                                                             |         |
| threshold       | *float* (min) or *range(float)* (min and max)                | Same as *[Gesture](#gesture).threshold*, but only for this action.<br><br>*Begin* actions can't have thresholds.                                                                                                                                                                                                                                                                                                                                                     |         |

Unlike gestures, the action type is determined only by the presence of one of the following properties.

### CommandAction : [Action](#action)
| Property    | Type     | Description    |
|-------------|----------|----------------|
| **command** | *string* | Run a command. |

### InputAction : [Action](#action)
| Property        | Type           | Description                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      |
|-----------------|----------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| **input**       |                | List of input devices and the actions to be performed by them. Devices can be used multiple times.<br><br>**Devices**<br>*keyboard*, *mouse*<br><br>**Keyboard and mouse actions**<br>``+[key/button]`` - Press *key* on keyboard or *button* on mouse<br>``-[key/button]`` - Release *key* on keyboard or *button* on mouse<br>``[key1/button1]+[key2/button2]`` - One or more keys/buttons separated by ``+``. Pressed in the order as specified and released in reverse order.<br>Full list of keys and buttons: [src/libinputactions/libinputactions/yaml_convert.h](../src/libinputactions/libinputactions/yaml_convert.h)<br><br>**Mouse actions**<br>``move_by [x] [y]`` - Move the pointer by *(x, y)*<br>``move_to [x] [y]`` - Move the pointer to *(x, y)*<br>``move_by_delta`` - Move the pointer by the gesture's delta. Swipe gestures have a different acceleration profile. The delta will be multiplied by *Device.delta_multiplier*.<br><br>Example:<br>``input:``<br>``  - keyboard: [ leftctrl+n ]``<br>`` - mouse: [ left ]``<br><br>**Mutually exclusive with *keyboard*.** |
| ~~keyboard~~    | *list(string)* | Like *input* but only for the keyboard.<br><br>**Deprecated. This option is kept for backwards compatibility and may be removed in the future.**<br><br>**Mutually exclusive with *input*.**                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     |

### PlasmaShortcutAction : [Action](#action)
| Property            | Type     | Description                                                                                                                                                                                                                                                                                                                                                                                 |
|---------------------|----------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| **plasma_shortcut** | *string* | Invoke a KDE Plasma global shortcut. Format: ``component,shortcut``.<br><br>Run ``qdbus org.kde.kglobalaccel \| grep /component`` for the list of components. Don't put the */component/* prefix in this file.<br>Run ``qdbus org.kde.kglobalaccel /component/$component org.kde.kglobalaccel.Component.shortcutNames`` for the list of shortcuts.<br><br>Example: ``kwin,Window Minimize`` |

## ActionGroup
Groups control how actions are executed. Actions inside groups ignore the *on*, *interval* and *threshold* properties. Those properties should be set on the group itself. Conditions are allowed.

Like actions, the group type is determined by the presence of one of the following properties.

### OneActionGroup : [ActionGroup](#actiongroup)
| Property | Type                      | Description                                          |
|----------|---------------------------|------------------------------------------------------|
| **one**  | *list([Action](#action))* | Executes only the first action that can be executed. |

#### Example
```yaml
# Exit fullscreen if fullscreen, unmaximize if maximized and minimize otherwise
one:
  - plasma_shortcut: kwin,Window Fullscreen
    conditions:
      - $window_fullscreen == true

  - plasma_shortcut: kwin,Window Maximize
    conditions:
      - $window_maximized == true

  - plasma_shortcut: kwin,Window Minimize
```

# Advanced
## YAML Anchors
Anchors can be used to reduce duplication.

``&name`` - Define (must be defined before referencing) <br>
``*name`` - Reference

The merge operator ``<<`` is not supported, therefore this only works well with simple types.
 
### Example

```yaml
call_this_whatever_you_want:
  - &touchpad_interval_p 75
  - &touchpad_interval_n -75
  
touchpad:
  gestures:
    - type: swipe
      direction: up

      actions:
        - on: update
          interval: *touchpad_interval_n
          # ...
          
        - on: update
          interval: *touchpad_interval_p
          # ...
```

## GestureGroup
Gesture groups apply all properties except *gestures* to the gestures specified in the *gestures* property. This can also be used to reduce duplication.

### Example
```yaml
touchpad:
  gestures:
    # Firefox gestures
    - conditions:
        - $window_class == firefox

      gestures:
        # Firefox swipe gestures with meta modifier
        - type: swipe
          conditions:
            - $keyboard_modifiers == meta
  
          gestures:
            - direction: right
              conditions:
                - $pointer_position_window_percentage_x < 0.5
              
            - direction: left
              conditions:
                - $pointer_position_window_percentage_x >= 0.5

        # Firefox swipe gestures with alt modifier
        - type: swipe
          conditions:
            - $keyboard_modifiers == alt

          gestures:
            - direction: right
              conditions:
                - $pointer_position_window_percentage_x < 0.5

            - direction: left
              conditions:
                - $pointer_position_window_percentage_x >= 0.5
```

# Configuration example
```yaml
mouse:
  - whitelist: [ 'TPPS/2 IBM TrackPoint' ]
    gestures:
      # ...

  # Input events from 'TPPS/2 IBM TrackPoint' will not reach the handler below.
  - gestures:
      # ...

touchpad:
  speed:
    swipe_threshold: 15

  gestures:
    - type: pinch
      fingers: 2
      direction: in

      actions:
        - plasma_shortcut: kwin,Window Close
```
