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

#include "MultiTouchMotionTriggerHandler.h"
#include <libinputactions/input/devices/InputDeviceState.h>

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
 * Event filtering requires blocking events by default until a gesture is recognized. The device's virtual state is managed by this handler. The input backend
 * must not do anything else other than blocking individual events.
 *
 * Can handle one device. Each device has its own instance.
 */
class TouchscreenTriggerHandler : public MultiTouchMotionTriggerHandler
{
public:
    TouchscreenTriggerHandler(InputDevice *device);

protected:
    bool evdevFrame(const EvdevFrameEvent &event) override;
    bool touchCancel(const TouchCancelEvent &event) override;
    bool touchDown(const TouchDownEvent &event) override;
    bool touchFrame(const TouchFrameEvent &event) override;
    bool touchMotion(const TouchMotionEvent &event) override;
    bool touchUp(const TouchUpEvent &event) override;

private slots:
    void onHoldTimerTimeout();
    void onTouchDownTimerTimeout();

private:
    void handleTouchUp();
    void handleTap();

    void beginGestureRecognition();
    void setBlockAndUpdateVirtualDeviceState(bool value);

    /**
     * Initial point positions for gesture recognition. May be different than the actual initial position. Key is the point id.
     */
    std::map<int32_t, QPointF> m_pointInitialPositions;
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

    enum class State
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
    void setState(State state);

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