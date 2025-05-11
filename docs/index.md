# Documentation
[Configuration](configuration.md)<br>
[Example gestures](example_gestures.md)

# Gestures
## Lifecycle
- activate - If a gesture's activation conditions are satisfied, it is added to the list of active gestures and can receive update events. If one of the activated gestures has set speed, all of them will only receive events after it is determined (*Speed.events*)
- begin - A gesture usually begins on the first update event, but that can be delayed or prevented entirely by thresholds.
- update - Before the gesture receives an update event, its update conditions (direction) are checked first. If not satisfied, the gesture is cancelled.
- end - The gesture has ended and end conditions have been satisfied.
- cancel - The gesture's update conditions were not satisfied, the finger count has changed (touchpad), or there is a gesture conflict (see section below). Cancel actions are only executed if the gesture had begun.

## Conflict resolution
All active gestures receive events. This can lead to them conflicting with each other. To prevent this, all other gestures are cancelled immediately when:
- a gesture is updated, and has any action that had been executed, or has an update action that can be executed but hasn't been yet (interval not reached)
- a gesture ends, and has an end action that can be executed
- a stroke gesture is active (in this case only swipe gestures are cancelled)

Gestures receive update events in order as specified in the configuration file.

In the following example, it is not necessary to set ``speed: slow`` on *Gesture 2* anymore, because when the action of *Gesture 1* is executed, *Gesture 2* will be cancelled immediately.
```yaml
# Gesture 1
- type: swipe
  direction: right
  fingers: 3
  speed: fast
  
  actions:
    - on: begin
      # ...

# Gesture 2
- type: swipe
  direction: right
  fingers: 3

  actions:
    - on: begin
      # ...
```

## Press gestures
Press gestures update every 5 ms with a delta of 5.

## Swipe gestures
Swipe gestures are limited to 4 directions. The direction is determined during the first few input events, which allows for executing actions at all points of the gesture's lifecycle.

## Stroke gestures
Stroke gestures allow you to draw any shape. The performed stroke is compared against all gestures and the one with the highest match (must be ≥70%) is ended, while all others are cancelled. 

Strokes can be recorded using the stroke recorder at *System Settings* -> *Desktop Effects* -> *Input Actions (configure)* or DBus: ``qdbus org.inputactions / recordStroke``.

Only *end* actions are supported.

# Input
Input events are sent and blocked at compositor level. Programs that read from /dev/input may not function as expected.  

# Devices
## Mouse
Mouse gestures are only supported on Plasma 6.3

### Press gestures
Mouse press gestures do not start immediately by default, allowing swipe gestures or normal clicks to be performed. 

If this behavior is not desired, *PressGesture.instant* should be set to true. The property is set per-gesture, but affects all activated press gestures.

### Wheel gestures
If a mouse button or keyboard modifier is pressed, wheel gestures end only after it is released. If none are pressed, gestures begin, update and end instantly on every single scroll event and therefore do not support thresholds.

### Distinguishing between normal clicks and press/swipe gestures
When a mouse button is pressed, the event is blocked and the plugin gives the user 200 ms to perform any mouse movement. After that period, a press gesture is started, however if none were activated, all previously blocked buttons are pressed.

## Touchpad
### Acceleration
KWin only provides unaccelerated deltas for both scroll and swipe events.

### Edge detection
This is not possible due to KWin not providing the absolute positions of fingers. Gesture speed and min/max thresholds may be used as an alternative.

### Press (hold) gestures
Single- and two-finger press gestures begin almost immediately. Three- and four-finger gestures have a significant delay added by libinput that may make those gestures annoying to use.

### Two-finger swipe gestures
Two-finger swipe gestures are achieved by treating scroll events as swipe events. As a result of this approach, the plugin does not know when fingers are lifted, so it must assume that the gesture has ended if there have been no events for 100 ms.

Another issue is that the thresholds for changing the scroll axis are quite large, which can cause problems with stroke gestures and ``direction: any`` swipe gestures utilizing the ``move_by_delta`` mouse input action.

If scrolling on touchpad edges is enabled, this feature will not work.