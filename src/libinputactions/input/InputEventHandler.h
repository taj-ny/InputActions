/*
    Input Actions - Input handler that executes user-defined actions
    Copyright (C) 2024-2026 Marcin Woźniak

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

namespace InputActions
{

class EvdevFrameEvent;
class KeyboardKeyEvent;
class InputEvent;
class MotionEvent;
class PointerButtonEvent;
class TouchCancelEvent;
class TouchDownEvent;
class TouchFrameEvent;
class TouchMotionEvent;
class TouchPressureChangeEvent;
class TouchUpEvent;
class TouchpadClickEvent;
class TouchpadGestureLifecyclePhaseEvent;
class TouchpadPinchEvent;

/**
 * All methods return whether the event should be blocked.
 */
class InputEventHandler
{
public:
    virtual ~InputEventHandler() = default;

    /**
     * Forwards the specified event to the appropriate virtual method if acceptsEvent returns true.
     */
    bool handleEvent(const InputEvent &event);

protected:
    InputEventHandler() = default;

    virtual bool acceptsEvent(const InputEvent &event);

    virtual bool evdevFrame(const EvdevFrameEvent &event) { return false; }

    virtual bool keyboardKey(const KeyboardKeyEvent &event) { return false; }

    virtual bool pointerAxis(const MotionEvent &event) { return false; }
    virtual bool pointerButton(const PointerButtonEvent &event) { return false; }
    virtual bool pointerMotion(const MotionEvent &event) { return false; }

    virtual bool touchCancel(const TouchCancelEvent &event) { return false; }
    virtual bool touchDown(const TouchDownEvent &event) { return false; }
    virtual bool touchFrame(const TouchFrameEvent &event) { return false; }
    virtual bool touchMotion(const TouchMotionEvent &event) { return false; }
    virtual bool touchPressureChange(const TouchPressureChangeEvent &event) { return false; }
    virtual bool touchUp(const TouchUpEvent &event) { return false; }

    virtual bool touchpadClick(const TouchpadClickEvent &event) { return false; }
    virtual bool touchpadGestureLifecyclePhase(const TouchpadGestureLifecyclePhaseEvent &event) { return false; }
    virtual bool touchpadPinch(const TouchpadPinchEvent &event) { return false; }
    virtual bool touchpadSwipe(const MotionEvent &event) { return false; }
};

}