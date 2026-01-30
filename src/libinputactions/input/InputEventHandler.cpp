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

#include "InputEventHandler.h"
#include "events.h"

namespace InputActions
{

bool InputEventHandler::handleEvent(const InputEvent &event)
{
    if (!acceptsEvent(event)) {
        return false;
    }

    switch (event.type()) {
        case InputEventType::EvdevFrame:
            return evdevFrame(dynamic_cast<const EvdevFrameEvent &>(event));
        case InputEventType::KeyboardKey:
            return keyboardKey(dynamic_cast<const KeyboardKeyEvent &>(event));
        case InputEventType::PointerAxis:
            return pointerAxis(dynamic_cast<const MotionEvent &>(event));
        case InputEventType::PointerButton:
            return pointerButton(dynamic_cast<const PointerButtonEvent &>(event));
        case InputEventType::PointerMotion:
            return pointerMotion(dynamic_cast<const MotionEvent &>(event));
        case InputEventType::TouchCancel:
            return touchCancel(dynamic_cast<const TouchCancelEvent &>(event));
        case InputEventType::TouchDown:
            return touchDown(dynamic_cast<const TouchDownEvent &>(event));
        case InputEventType::TouchFrame:
            return touchFrame(dynamic_cast<const TouchFrameEvent &>(event));
        case InputEventType::TouchMotion:
            return touchMotion(dynamic_cast<const TouchMotionEvent &>(event));
        case InputEventType::TouchPressureChange:
            return touchPressureChange(dynamic_cast<const TouchPressureChangeEvent &>(event));
        case InputEventType::TouchUp:
            return touchUp(dynamic_cast<const TouchUpEvent &>(event));
        case InputEventType::TouchpadClick:
            return touchpadClick(dynamic_cast<const TouchpadClickEvent &>(event));
        case InputEventType::TouchpadGestureLifecyclePhase:
            return touchpadGestureLifecyclePhase(dynamic_cast<const TouchpadGestureLifecyclePhaseEvent &>(event));
        case InputEventType::TouchpadSwipe:
            return touchpadSwipe(dynamic_cast<const MotionEvent &>(event));
        case InputEventType::TouchpadPinch:
            return touchpadPinch(dynamic_cast<const TouchpadPinchEvent &>(event));
        default:
            return false;
    }
}

bool InputEventHandler::acceptsEvent(const InputEvent &event)
{
    return true;
}

}