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
/*
    Parts of the gesture recognition code were taken from libinput.
    https://gitlab.freedesktop.org/libinput/libinput

    Copyright © 2006-2009 Simon Thum
    Copyright © 2008-2012 Kristian Høgsberg
    Copyright © 2010-2012 Intel Corporation
    Copyright © 2010-2011 Benjamin Franzke
    Copyright © 2011-2012 Collabora, Ltd.
    Copyright © 2013-2014 Jonas Ådahl
    Copyright © 2013-2015 Red Hat, Inc.

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice (including the next
    paragraph) shall be included in all copies or substantial portions of the
    Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
 */

#include "TouchscreenTriggerHandler.h"
#include "interfaces/InputEmitter.h"
#include <libinputactions/input/events.h>

namespace InputActions
{

static const auto MOTION_THRESHOLD_MM = 4;
static const std::chrono::milliseconds HOLD_TIMEOUT{200L};
static const std::chrono::milliseconds TAP_TIMEOUT{200L};
static const std::chrono::milliseconds TOUCH_DOWN_TIMEOUT{50L};
static const std::chrono::milliseconds TOUCH_UP_TIMEOUT{50L};

TouchscreenTriggerHandler::TouchscreenTriggerHandler(InputDevice *device)
{
    setDevice(device);

    m_holdTimer.setTimerType(Qt::PreciseTimer);
    m_holdTimer.setSingleShot(true);
    connect(&m_holdTimer, &QTimer::timeout, this, &TouchscreenTriggerHandler::onHoldTimerTimeout);

    m_touchDownTimer.setTimerType(Qt::PreciseTimer);
    m_touchDownTimer.setSingleShot(true);
    connect(&m_touchDownTimer, &QTimer::timeout, this, &TouchscreenTriggerHandler::onTouchDownTimerTimeout);

    m_touchUpTimer.setTimerType(Qt::PreciseTimer);
    m_touchUpTimer.setSingleShot(true);
    connect(&m_touchUpTimer, &QTimer::timeout, this, &TouchscreenTriggerHandler::handleTouchUp);
}

bool TouchscreenTriggerHandler::evdevFrame(const EvdevFrameEvent &event)
{
    // Block events that don't map to InputActions events (e.g. pressure change)
    switch (m_state) {
        case State::None:
            setBlockAndUpdateVirtualDeviceState(true); // Block by default
            break;
    }
    return m_block;
}

bool TouchscreenTriggerHandler::touchCancel(const TouchCancelEvent &event)
{
    switch (m_state) {
        case State::Hold:
        case State::Pinch:
        case State::Swipe:
            cancelTriggers(TriggerType::All);
            break;
    }

    const auto block = m_block;
    setState(State::None);
    return block;
}

bool TouchscreenTriggerHandler::touchChanged(const TouchChangedEvent &event)
{
    m_touchModifiedInCurrentFrame = true;
    return m_block;
}

bool TouchscreenTriggerHandler::touchDown(const TouchEvent &event)
{
    switch (m_state) {
        case State::WaitingForTouchDowns:
            break;
        default:
            setState(State::WaitingForTouchDowns);
            break;
    }

    setBlockAndUpdateVirtualDeviceState(true); // Block by default
    beginGestureRecognition();
    updateVariables(m_device);

    m_preTouchUpPoints.clear();
    for (const auto *point : event.sender()->validTouchPoints()) {
        m_preTouchUpPoints.push_back(*point);
    }

    return true;
}

bool TouchscreenTriggerHandler::touchFrame(const TouchFrameEvent &event)
{
    if (!m_touchModifiedInCurrentFrame) {
        const auto block = m_block || m_blockNextFrame;
        m_blockNextFrame = false;
        return block;
    }
    m_touchModifiedInCurrentFrame = false;

    const auto &points = event.sender()->validTouchPoints();
    for (const auto *point : points) {
        for (auto &preTouchUpPoint : m_preTouchUpPoints) {
            if (point->id == preTouchUpPoint.id) {
                preTouchUpPoint.position = point->position;
                preTouchUpPoint.unalteredPosition = point->unalteredPosition;
            }
        }
    }

    if (m_state != State::WaitingForTouchUps) {
        updateVariables(m_device);
    }

    switch (m_state) {
        case State::Pinch: {
            const auto info = pinchInfo();
            const auto scale = info.distance / m_initialDistance;

            auto angleDelta = info.angle - m_previousAngle;
            if (angleDelta > 180.0) {
                angleDelta -= 360.0;
            } else if (angleDelta < -180.0) {
                angleDelta += 360.0;
            }
            m_previousAngle = info.angle;

            setBlockAndUpdateVirtualDeviceState(handlePinch(scale, angleDelta));

            break;
        }
        case State::Swipe: {
            const auto center = touchCenter();
            setBlockAndUpdateVirtualDeviceState(handleMotion(m_device, center - m_previousCenter));
            m_previousCenter = center;
            break;
        }
        case State::WaitingForTouchDowns:
        case State::Hold:
        case State::Touch: {
            const auto fingerPassedThreshold = [this](const auto *point) {
                return hypot(point->position - m_pointInitialPositions[point]) >= MOTION_THRESHOLD_MM;
            };
            if (std::ranges::any_of(points, fingerPassedThreshold)) {
                setState(State::MotionOnePointReachedThreshold);
            } else {
                break;
            }
            [[fallthrough]];
        }
        case State::MotionOnePointReachedThreshold: {
            const auto fingerPassedThreshold = [this](const auto *point) {
                return hypot(point->position - m_pointInitialPositions[point]) >= MOTION_THRESHOLD_MM;
            };
            if (std::ranges::all_of(points, fingerPassedThreshold)) {
                setState(State::Motion);
            } else {
                break;
            }
            [[fallthrough]];
        }
        case State::Motion:
            bool sameDirection = true;
            uint32_t firstDirection{};
            QPointF totalDelta;
            for (size_t i = 0; i < points.size(); i++) {
                totalDelta += m_pointInitialPositions[points[i]] - points[i]->position;
                const auto direction = directionFromPoint(m_pointInitialPositions[points[i]] - points[i]->position);
                if (i == 0) {
                    firstDirection = direction;
                    continue;
                }

                if (!sameDirections(firstDirection, direction)) {
                    sameDirection = false;
                    break;
                }
            }
            if (sameDirection) {
                setState(State::Swipe);

                handleMotion(m_device, totalDelta / points.size());
                break;
            }

            setState(State::Pinch);
            break;
    }

    const auto block = m_block || m_blockNextFrame;
    m_blockNextFrame = false;
    return block;
}

bool TouchscreenTriggerHandler::touchUp(const TouchEvent &event)
{
    switch (m_state) {
        case State::WaitingForTouchUps:
            if (m_device->validTouchPoints().empty()) {
                m_blockNextFrame = m_block;
                handleTouchUp();
            }
            break;
        default:
            if (m_device->validTouchPoints().empty()) {
                m_blockNextFrame = m_block;
                handleTouchUp();
                break;
            }

            setState(State::WaitingForTouchUps);
            break;
    }

    return m_block;
}

void TouchscreenTriggerHandler::onHoldTimerTimeout()
{
    switch (m_state) {
        case State::Touch:
            setState(State::Hold);
            break;
    }
}

void TouchscreenTriggerHandler::onTouchDownTimerTimeout()
{
    setState(State::Touch);
    if (blockingTriggers(TriggerType::All, {}).empty()) {
        setBlockAndUpdateVirtualDeviceState(false);
    }
}

void TouchscreenTriggerHandler::handleTouchUp()
{
    if (!m_device->validTouchPoints().empty()) {
        cancelTriggers(TriggerType::All);
        beginGestureRecognition();
        updateVariables(m_device);
        setState(State::Touch);
        return;
    }

    switch (m_state) {
        case State::WaitingForTouchDowns:
        case State::WaitingForTouchUps:
        case State::Touch: {
            const auto now = std::chrono::steady_clock::now();
            if (std::ranges::all_of(m_preTouchUpPoints, [this, &now](const auto &point) {
                    return std::chrono::duration_cast<std::chrono::milliseconds>(now - point.downTimestamp) <= TAP_TIMEOUT
                        && hypot(point.position - point.initialPosition) < MOTION_THRESHOLD_MM;
                })) {
                handleTap();
            }
            break;
        }
    }
    setState(State::None);
    updateVariables();
}

void TouchscreenTriggerHandler::handleTap()
{
    const auto result = activateTriggers(TriggerType::Tap);
    if (result.success) {
        updateTriggers(TriggerType::Tap);
        endTriggers(TriggerType::Tap);
    }

    if (!result.block && m_block) {
        std::vector<QPointF> points;
        for (const auto &point : m_preTouchUpPoints) {
            points.push_back(point.unalteredPosition);
        }
        m_device->simulateTouchscreenTap(points);
    }
}

void TouchscreenTriggerHandler::setState(State state)
{
    // State exit
    switch (m_state) {
        case State::Touch:
            m_holdTimer.stop();
            break;
        case State::WaitingForTouchDowns:
            m_holdTimer.stop();
            m_touchDownTimer.stop();
            break;
        case State::WaitingForTouchUps:
            m_touchUpTimer.stop();
            break;
    }

    // State transition
    switch (m_state) {
        case State::Hold:
        case State::Pinch:
        case State::Swipe:
            switch (state) {
                case State::None:
                    endTriggers(TriggerType::All);
                    break;
                default:
                    cancelTriggers(TriggerType::All);
                    break;
            }
            break;
    }

    // State enter
    switch (state) {
        case State::Hold:
            if (!activateTriggers(TriggerType::Press).block && m_block) {
                setBlockAndUpdateVirtualDeviceState(false);
            }
            break;
        case State::Pinch: {
            const auto pinch = pinchInfo();
            m_initialDistance = pinch.distance;
            ;
            m_previousAngle = pinch.angle;

            setBlockAndUpdateVirtualDeviceState(activateTriggers(TriggerType::PinchRotate).block);
            break;
        }
        case State::Swipe:
            m_previousCenter = touchCenter();
            setBlockAndUpdateVirtualDeviceState(activateTriggers(TriggerType::SinglePointMotion).block);
            break;
        case State::Touch:
            m_holdTimer.start(HOLD_TIMEOUT);
            break;
        case State::WaitingForTouchDowns:
            m_holdTimer.start(HOLD_TIMEOUT);
            m_touchDownTimer.start(TOUCH_DOWN_TIMEOUT);
            break;
        case State::WaitingForTouchUps:
            m_touchUpTimer.start(TOUCH_UP_TIMEOUT);
            break;
    }

    m_state = state;
}

void TouchscreenTriggerHandler::beginGestureRecognition()
{
    const auto points = m_device->validTouchPoints();
    m_initialDistance = {};
    m_previousAngle = {};

    m_pointInitialPositions.clear();
    for (const auto &point : points) {
        m_pointInitialPositions[point] = point->position;
    }
}

void TouchscreenTriggerHandler::setBlockAndUpdateVirtualDeviceState(bool value)
{
    if (m_state != State::None) {
        if (value && !m_block) {
            m_device->resetVirtualDeviceState();
        } else if (!value && m_block) {
            m_device->restoreVirtualDeviceState();
        }
    }
    m_block = value;
}

PinchInfo TouchscreenTriggerHandler::pinchInfo() const
{
    const auto points = m_device->validTouchPoints();

    const auto &first = points[0];
    const auto &second = points[1];
    const auto delta = first->position - second->position;

    return {
        .angle = radiansToDegrees(atan2(delta)),
        .distance = hypot(delta),
    };
}

QPointF TouchscreenTriggerHandler::touchCenter() const
{
    QPointF center;
    const auto points = m_device->validTouchPoints();
    for (const auto *point : m_device->validTouchPoints()) {
        center += point->position;
    }
    center /= points.size();
    return center;
}

uint32_t TouchscreenTriggerHandler::directionFromPoint(const QPointF &point)
{
    enum Direction
    {
        N = 1u,
        NE = 1u << 1,
        E = 1u << 2,
        SE = 1u << 3,
        S = 1u << 4,
        SW = 1u << 5,
        W = 1u << 6,
        NW = 1u << 7,
        UNDEFINED_DIRECTION = 0xff
    };

    const auto x = point.x();
    const auto y = point.y();

    uint32_t dir = UNDEFINED_DIRECTION;
    int d1{};
    int d2{};
    double r{};

    if (fabs(x) < 2.0 && fabs(y) < 2.0) {
        if (x > 0.0 && y > 0.0)
            dir = S | SE | E;
        else if (x > 0.0 && y < 0.0)
            dir = N | NE | E;
        else if (x < 0.0 && y > 0.0)
            dir = S | SW | W;
        else if (x < 0.0 && y < 0.0)
            dir = N | NW | W;
        else if (x > 0.0)
            dir = NE | E | SE;
        else if (x < 0.0)
            dir = NW | W | SW;
        else if (y > 0.0)
            dir = SE | S | SW;
        else if (y < 0.0)
            dir = NE | N | NW;
    } else {
        /* Calculate r within the interval  [0 to 8)
         *
         * r = [0 .. 2π] where 0 is North
         * d_f = r / 2π  ([0 .. 1))
         * d_8 = 8 * d_f
         */
        r = std::atan2(y, x);
        r = fmod(r + 2.5 * M_PI, 2 * M_PI);
        r *= 4 * M_1_PI;

        /* Mark one or two close enough octants */
        d1 = static_cast<int>(r + 0.9) % 8;
        d2 = static_cast<int>(r + 0.1) % 8;

        dir = 1u << d1 | 1u << d2;
    }
    return dir;
}

bool TouchscreenTriggerHandler::sameDirections(uint32_t a, uint32_t b)
{
    /*
     * In some cases (semi-mt touchpads) we may seen one finger move
     * e.g. N/NE and the other W/NW so we not only check for overlapping
     * directions, but also for neighboring bits being set.
     * The ((dira & 0x80) && (dirb & 0x01)) checks are to check for bit 0
     * and 7 being set as they also represent neighboring directions.
     */
    return ((a | (a >> 1)) & b) || ((b | (b >> 1)) & a) || ((a & 0x80) && (b & 0x01)) || ((b & 0x80) && (a & 0x01));
}

double TouchscreenTriggerHandler::hypot(const QPointF &point)
{
    return std::hypot(point.x(), point.y());
}

double TouchscreenTriggerHandler::atan2(const QPointF &point)
{
    return std::atan2(point.y(), point.x());
}

double TouchscreenTriggerHandler::radiansToDegrees(double radians)
{
    return 180.0 / M_PI * radians;
}

}