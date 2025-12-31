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

#include "MultiTouchMotionTriggerHandler.h"

namespace InputActions
{

struct PinchInfo
{
    qreal angle;
    qreal distance;
};

/**
 * Handles touchscreen triggers: hold, pinch, rotate, single-point motion, tap.
 *
 * Can handle one device. Each device has its own instance.
 *
 * Manages the output device's state, the input backend must not do anything else other than blocking individual events.
 */
class TouchscreenTriggerHandler : public MultiTouchMotionTriggerHandler
{
public:
    TouchscreenTriggerHandler(InputDevice *device);

protected:
    bool touchCancel(const TouchCancelEvent &event) override;
    bool touchChanged(const TouchChangedEvent &event) override;
    bool touchDown(const TouchEvent &event) override;
    bool touchFrame(const TouchFrameEvent &event) override;
    bool touchUp(const TouchEvent &event) override;

private slots:
    void onHoldTimerTimeout();
    void onTouchDownTimerTimeout();

private:
    TEST_VIRTUAL void handleTouchUp();
    TEST_VIRTUAL void handleTap();

    TEST_VIRTUAL void beginGestureRecognition();
    void setBlockAndUpdateOutputDeviceState(bool value);

    /**
     * Initial point positions for gesture recognition. May be different than the actual initial position. Reset on touch up.
     */
    std::map<const TouchPoint *, QPointF> m_pointInitialPositions;
    QTimer m_holdTimer;

    QTimer m_touchDownTimer;
    QTimer m_touchUpTimer;
    std::vector<TouchPoint> m_preTouchUpPoints;

    qreal m_initialDistance;
    qreal m_previousAngle;
    QPointF m_previousCenter;

    bool m_touchModifiedInCurrentFrame{};

    bool m_block{};
    bool m_blockNextFrame{};

    enum State
    {
        None,

        WaitingForTouchDowns,
        WaitingForTouchUps,

        Touch,

        MotionOnePointReachedThreshold,
        Motion,

        Hold,
        Pinch,
        Swipe,
    } m_state
        = State::None;
    TEST_VIRTUAL void setState(State state);

    PinchInfo pinchInfo() const;
    QPointF touchCenter() const;

    static uint32_t directionFromPoint(const QPointF &point);
    static bool sameDirections(uint32_t a, uint32_t b);
    static double hypot(const QPointF &point);
    static double atan2(const QPointF &point);
    /**
     * @returns [0°, 360°]
     */
    static double radiansToDegrees(double radians);

    friend class MockTouchscreenTriggerHandler;
    friend class TestTouchscreenTriggerHandler;
};

}