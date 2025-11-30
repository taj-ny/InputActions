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

namespace InputActions
{

/**
 * Handles touchpad triggers: click, pinch, press, rotate, stroke, swipe, tap.
 *
 * Can handle one device. Each device has its own instance.
 */
class TouchpadTriggerHandler : public MultiTouchMotionTriggerHandler
{
public:
    TouchpadTriggerHandler(InputDevice *device);

    /**
     * The time for the user to perform a click once a press gesture had been detected by libinput. If the click is not performed, the press trigger is
     * activated.
     */
    std::chrono::milliseconds m_clickTimeout{200};

protected:
    /**
     * Treated as two-finger motion.
     */
    bool pointerAxis(const MotionEvent &event) override;
    bool pointerButton(const PointerButtonEvent &event) override;
    /**
     * Treated as single-finger motion.
     */
    bool pointerMotion(const MotionEvent &event) override;

    bool touchpadClick(const TouchpadClickEvent &event) override;
    bool touchpadGestureLifecyclePhase(const TouchpadGestureLifecyclePhaseEvent &event) override;
    bool touchpadPinch(const TouchpadPinchEvent &event) override;
    bool touchpadSwipe(const MotionEvent &event) override;

    void setState(State state) override;

private slots:
    void onLibinputTapTimeout();

private:
    std::set<uint32_t> m_blockedButtons;
    bool m_gestureBeginBlocked{};

    QTimer m_clickTimeoutTimer;
    QTimer m_libinputTapTimeoutTimer;

    bool m_previousPointerAxisEventBlocked{};
    PointDelta m_pointerAxisDelta;

    friend class MockTouchpadTriggerHandler;
    friend class TestTouchpadTriggerHandler;
};

}