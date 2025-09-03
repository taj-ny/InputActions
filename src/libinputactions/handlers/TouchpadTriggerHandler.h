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

#include <chrono>
#include <libinputactions/handlers/MultiTouchMotionTriggerHandler.h>

namespace libinputactions
{

/**
 * Handles touchpad triggers: click, pinch, press, rotate, stroke, swipe, tap.
 */
class TouchpadTriggerHandler : public MultiTouchMotionTriggerHandler
{
public:
    TouchpadTriggerHandler();

    bool handleEvent(const InputEvent &event) override;

    /**
     * The time for the user to perform a click once a press gesture had been detected by libinput. If the click is not performed, the press trigger is
     * activated.
     */
    std::chrono::milliseconds m_clickTimeout{200};

private:
    /**
     * Treated as single-finger motion.
     */
    bool handleEvent(const MotionEvent &event);
    bool handleEvent(const PointerButtonEvent &event);
    void handleEvent(const TouchpadClickEvent &event);
    bool handleEvent(const TouchpadGestureLifecyclePhaseEvent &event);
    bool handleEvent(const TouchpadPinchEvent &event);
    /**
     * Treated as two-finger motion.
     */
    bool handleScrollEvent(const MotionEvent &event);
    bool handleSwipeEvent(const MotionEvent &event);

    std::set<uint32_t> m_blockedButtons;
    bool m_gestureBeginBlocked{};

    QTimer m_clickTimeoutTimer;

    friend class TestTouchpadTriggerHandler;
};

}