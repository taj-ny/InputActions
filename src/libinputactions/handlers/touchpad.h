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

#include <libinputactions/handlers/multitouchmotion.h>

namespace libinputactions
{

/**
 * Handles touchpad triggers: pinch, press, rotate, stroke, swipe.
 * This handler follows libinput's gesture lifecycle, making it not possible to for example ignore finger count
 * changes.
 */
class TouchpadTriggerHandler : public MultiTouchMotionTriggerHandler
{
public:
    TouchpadTriggerHandler();

    bool handleEvent(const InputEvent *event) override;

    /**
     * The time of inactivity in milliseconds after which 2-finger motion triggers will end.
     */
    void setScrollTimeout(const uint32_t &timeout);

private:
    bool handleEvent(const TouchpadGestureLifecyclePhaseEvent *event);
    bool handleEvent(const TouchpadPinchEvent *event);
    /**
     * The event is treated as a 2-finger swipe. Will not work if edge scrolling is enabled. The handler is not aware
     * when the finger count changes, therefore it relies on a timeout to end triggers.
     * @see setScrollTimeout
     */
    bool handleScrollEvent(const MotionEvent *event);
    bool handleSwipeEvent(const MotionEvent *event);

    uint32_t m_scrollTimeout = 100;
    QTimer m_scrollTimeoutTimer;
};

}