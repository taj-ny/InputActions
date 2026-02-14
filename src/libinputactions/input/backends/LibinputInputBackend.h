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

#include <libinputactions/input/Delta.h>
#include <libinputactions/input/backends/LibevdevComplementaryInputBackend.h>

namespace InputActions
{

/**
 * Input backend that uses libinput events, but does not manage a libinput instance. Uses libevdev backend.
 */
class LibinputInputBackend : public LibevdevComplementaryInputBackend
{
protected:
    LibinputInputBackend() = default;

    /**
     * InputDevice::setKeyState must be called prior to this method.
     * @param sender The event will be ignored if nullptr.
     * @returns Whether to block the event.
     */
    bool keyboardKey(InputDevice *sender, KeyboardKey key, bool state);

    /**
     * Handles mouse wheel and touchpad scroll.
     * @param sender The event will be ignored if nullptr.
     * @returns Whether to block the event.
     */
    bool pointerAxis(InputDevice *sender, const QPointF &delta, bool oneAxisPerEvent = false);
    /**
     * Handles mouse and touchpad buttons.
     * @param sender The event will be ignored if nullptr.
     * @returns Whether to block the event.
     */
    bool pointerButton(InputDevice *sender, Qt::MouseButton button, uint32_t nativeButton, bool state);
    /**
     * @param sender The event will be ignored if nullptr.
     * @returns Whether to block the event.
     */
    bool pointerMotion(InputDevice *sender, const PointDelta &delta);

    /**
     * @param sender The event will be ignored if nullptr.
     * @returns Whether to block the event.
     */
    bool touchpadHoldBegin(InputDevice *sender, uint8_t fingers);
    /**
     * @param sender The event will be ignored if nullptr.
     * @returns Whether to block the event.
     */
    bool touchpadHoldEnd(InputDevice *sender, bool cancelled);

    /**
     * @param sender The event will be ignored if nullptr.
     * @returns Whether to block the event.
     */
    bool touchpadPinchBegin(InputDevice *sender, uint8_t fingers);
    /**
     * If the previous event (begin or update) was blocked but this one will not be, a pinch begin event will be emitted to allow the compositor/client to
     * handle the gesture.
     * @param sender The event will be ignored if nullptr.
     * @returns Whether to block the event.
     */
    bool touchpadPinchUpdate(InputDevice *sender, qreal scale, qreal angleDelta);
    /**
     * @param sender The event will be ignored if nullptr.
     * @returns Whether to block the event.
     */
    bool touchpadPinchEnd(InputDevice *sender, bool cancelled);

    /**
     * @param sender The event will be ignored if nullptr.
     * @returns Whether to block the event.
     */
    bool touchpadSwipeBegin(InputDevice *sender, uint8_t fingers);
    /**
     * If the previous event (begin or update) was blocked but this one will not be, a swipe begin event will be emitted to allow the compositor/client to
     * handle the gesture.
     * @param sender The event will be ignored if nullptr.
     * @returns Whether to block the event.
     */
    bool touchpadSwipeUpdate(InputDevice *sender, const PointDelta &delta);
    /**
     * @param sender The event will be ignored if nullptr.
     * @returns Whether to block the event.
     */
    bool touchpadSwipeEnd(InputDevice *sender, bool cancelled);

    bool touchscreenTouchCancel(InputDevice *sender);
    /**
     * @param position Physical position in millimeters relative to the top-left corner.
     * @param rawPosition Raw position provided by the compositor or evdev. Required for simulating taps.
     */
    bool touchscreenTouchDown(InputDevice *sender, int32_t id, const QPointF &position, const QPointF &rawPosition);
    bool touchscreenTouchFrame(InputDevice *sender);
    /**
     * @param position Physical position in millimeters relative to the top-left corner.
     * @param rawPosition Raw position provided by the compositor or evdev. Required for simulating taps.
     */
    bool touchscreenTouchMotion(InputDevice *sender, int32_t id, const QPointF &position, const QPointF &rawPosition);
    bool touchscreenTouchUp(InputDevice *sender, int32_t id);

    Qt::MouseButton scanCodeToMouseButton(uint32_t scanCode) const;

    /**
     * Called when a touchpad pinch update event was not blocked, but the previous one was.
     */
    virtual void touchpadPinchBlockingStopped(uint32_t fingers) {}
    /**
     * Called when a touchpad swipe update event was not blocked, but the previous one was.
     */
    virtual void touchpadSwipeBlockingStopped(uint32_t fingers) {}

private:
    uint32_t m_fingers{};
    bool m_block{};

    std::optional<QPointF> m_previousPointerPosition;
};

}