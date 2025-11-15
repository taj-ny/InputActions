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

#include <libinputactions/input/backends/LibevdevComplementaryInputBackend.h>
#include <libinputactions/input/Delta.h>

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
     * Keyboard::updateModifiers must be called prior to this method.
     * @param sender The event will be ignored if nullptr.
     * @returns Whether to block the event.
     */
    bool keyboardKey(InputDevice *sender, uint32_t key, bool state);

    /**
     * Handles mouse wheel and touchpad scroll.
     * @param sender The event will be ignored if nullptr.
     * @returns Whether to block the event.
     */
    bool pointerAxis(InputDevice *sender, const QPointF &delta);
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