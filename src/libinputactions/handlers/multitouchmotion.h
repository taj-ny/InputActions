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

#include "motion.h"

Q_DECLARE_LOGGING_CATEGORY(LIBINPUTACTIONS_HANDLER_MULTITOUCH)

namespace libinputactions
{

enum class PinchType
{
    Unknown,
    Pinch,
    Rotate
};

/**
 * Handles multi-touch triggers: pinch, rotate.
 * In the future this will also be able to recognize triggers based on touch points.
 */
class MultiTouchMotionTriggerHandler : public MotionTriggerHandler
{
protected:
    MultiTouchMotionTriggerHandler() = default;

    /**
     * Does nothing if there are no active pinch or rotate triggers.
     * @return Whether there are any active pinch or rotate triggers.
     */
    bool handlePinch(const qreal &scale, const qreal &angleDelta);

    void reset() override;

private:
    qreal m_previousPinchScale = 1;
    PinchType m_pinchType = PinchType::Unknown;
    qreal m_accumulatedRotateDelta = 0;
};

}