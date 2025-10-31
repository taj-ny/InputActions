/*
    Input Actions - Input handler that executes user-defined actions
    Copyright (C) 2024-2025 Marcin Woźniak

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

class KeyboardKeyEvent;
class InputEvent;
class MotionEvent;
class PointerButtonEvent;
class TouchChangedEvent;
class TouchEvent;
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

    virtual bool keyboardKey(const KeyboardKeyEvent &event) { return false; }

    virtual bool pointerAxis(const MotionEvent &event) { return false; }
    virtual bool pointerButton(const PointerButtonEvent &event) { return false; }
    virtual bool pointerMotion(const MotionEvent &event) { return false; }

    virtual bool touchChanged(const TouchChangedEvent &event) { return false; }
    virtual bool touchDown(const TouchEvent &event) { return false; }
    virtual bool touchUp(const TouchEvent &event) { return false; }

    virtual bool touchpadClick(const TouchpadClickEvent &event) { return false; }
    virtual bool touchpadGestureLifecyclePhase(const TouchpadGestureLifecyclePhaseEvent &event) { return false; }
    virtual bool touchpadPinch(const TouchpadPinchEvent &event) { return false; }
    virtual bool touchpadSwipe(const MotionEvent &event) { return false; }
};

}