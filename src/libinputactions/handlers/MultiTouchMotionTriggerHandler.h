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

#include "MotionTriggerHandler.h"
#include <chrono>

Q_DECLARE_LOGGING_CATEGORY(INPUTACTIONS_HANDLER_MULTITOUCH)

static const std::chrono::milliseconds TAP_TIMEOUT(200L);

namespace libinputactions
{

enum class PinchType
{
    Unknown,
    Pinch,
    Rotate
};

/**
 * Handles multi-touch triggers: pinch, tap, rotate.
 * In the future this will also be able to recognize triggers based on touch points.
 */
class MultiTouchMotionTriggerHandler : public MotionTriggerHandler
{
public:
    bool handleEvent(const InputEvent &event) override;

protected:
    MultiTouchMotionTriggerHandler() = default;

    /**
     * Does nothing if there are no active pinch or rotate triggers.
     * @return Whether there are any active pinch or rotate triggers.
     */
    bool handlePinch(qreal scale, qreal angleDelta);

    void reset() override;

    enum State
    {
        TouchpadButtonDown,
        /**
         * TouchpadButtonDown but a click trigger is active.
         */
        TouchpadButtonDownClickTrigger,

        None,
        Scrolling,

        /**
         * Finger(s) present but no action had been performed other than adding more fingers.
         */
        TouchIdle,
        /**
         * Finger(s) present and an action had been performed.
         */
        Touch,

        TapBegin
    } m_state
        = State::None;

private:
    void handleTouchDownEvent(const TouchEvent &event);
    void handleEvent(const TouchChangedEvent &event);
    void handleTouchUpEvent(const TouchEvent &event);

    bool canTap(const InputDevice *device);
    void updateVariables(const InputDevice *device);

    qreal m_previousPinchScale = 1;
    PinchType m_pinchType = PinchType::Unknown;
    qreal m_accumulatedRotateDelta = 0;

    friend class TestTouchpadTriggerHandler;
};

}