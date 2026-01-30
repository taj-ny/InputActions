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

#include <chrono>
#include <libinputactions/handlers/MultiTouchMotionTriggerHandler.h>
#include <libinputactions/input/devices/InputDeviceState.h>

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
    void setClickTimeout(std::chrono::milliseconds value) { m_clickTimeout = value; }

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

    bool touchDown(const TouchDownEvent &event) override;
    bool touchMotion(const TouchMotionEvent &event) override;
    bool touchUp(const TouchUpEvent &event) override;

    bool touchpadClick(const TouchpadClickEvent &event) override;
    bool touchpadGestureLifecyclePhase(const TouchpadGestureLifecyclePhaseEvent &event) override;
    bool touchpadPinch(const TouchpadPinchEvent &event) override;
    bool touchpadSwipe(const MotionEvent &event) override;

private slots:
    void onLibinputTapTimeout();

private:
    bool canTap();

    std::set<uint32_t> m_blockedButtons;
    bool m_gestureBeginBlocked{};

    QTimer m_clickTimeoutTimer;
    QTimer m_libinputTapTimeoutTimer;

    bool m_previousPointerAxisEventBlocked{};
    PointDelta m_pointerAxisDelta;

    std::chrono::milliseconds m_clickTimeout{200L};

    TouchPoint m_firstTouchPoint;

    enum State
    {
        TouchpadButtonDown,
        /**
         * TouchpadButtonDown but the press event was blocked.
         */
        TouchpadButtonDownBlocked,

        None,
        Scrolling,

        /**
         * Finger(s) present but no action had been performed other than adding more fingers.
         */
        TouchIdle,
        /**
         * Finger(s) present and an action had been performed (tap or click).
         */
        Touch,

        /**
         * At least one finger was moved.
         */
        Motion,
        /**
         * At least one finger was moved and no triggers were recognized.
         */
        MotionNoTrigger,
        /**
         * At least one finger was moved and a trigger was recognized.
         */
        MotionTrigger,

        /**
         * A tap gesture had been recognized and is being handled by InputActions.
         */
        TapBegin,
        /**
         * A tap gesture had been recognized and will be handled on libinput's pointer button event.
         */
        LibinputTapBegin
    } m_state
        = State::None;
    void setState(State state);

    friend class MockTouchpadTriggerHandler;
    friend class TestTouchpadTriggerHandler;
};

}