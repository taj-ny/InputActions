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

#include "StrokeRecorder.h"
#include "events.h"
#include <libinputactions/triggers/StrokeTrigger.h>

namespace InputActions
{

static const std::chrono::milliseconds STROKE_RECORD_TIMEOUT{250L};

StrokeRecorder::StrokeRecorder()
{
    m_strokeRecordingTimeoutTimer.setSingleShot(true);
    connect(&m_strokeRecordingTimeoutTimer, &QTimer::timeout, this, [this] {
        finishStrokeRecording();
    });
}

void StrokeRecorder::recordStroke(const std::function<void(const Stroke &stroke)> &callback)
{
    m_isRecordingStroke = true;
    m_strokeCallback = callback;
}

bool StrokeRecorder::evdevFrame(const EvdevFrameEvent &event)
{
    return (m_isRecordingStroke || m_blockTouchscreenEventsUntilDeviceNeutral) && event.sender()->type() == InputDeviceType::Touchscreen;
}

bool StrokeRecorder::pointerAxis(const MotionEvent &event)
{
    if (event.sender()->type() != InputDeviceType::Touchpad || !m_isRecordingStroke) {
        return false;
    }

    const auto delta = event.delta().unaccelerated();
    if (delta.isNull()) {
        finishStrokeRecording();
    } else {
        m_strokePoints.push_back(delta);
    }
    return true;
}

bool StrokeRecorder::pointerMotion(const MotionEvent &event)
{
    if (!m_isRecordingStroke) {
        return false;
    }

    m_strokePoints.push_back(event.delta().accelerated()); // accelerated for backwards compatibility
    m_strokeRecordingTimeoutTimer.start(STROKE_RECORD_TIMEOUT);
    return false;
}

bool StrokeRecorder::touchCancel(const TouchCancelEvent &event)
{
    return m_isRecordingStroke || m_blockTouchscreenEventsUntilDeviceNeutral;
}

bool StrokeRecorder::touchChanged(const TouchChangedEvent &event)
{
    return (m_isRecordingStroke || m_blockTouchscreenEventsUntilDeviceNeutral) && event.sender()->type() == InputDeviceType::Touchscreen;
}

bool StrokeRecorder::touchDown(const TouchEvent &event)
{
    m_previousTouchscreenTouchCenter = {};
    m_strokePoints.clear();
    return (m_isRecordingStroke || m_blockTouchscreenEventsUntilDeviceNeutral) && event.sender()->type() == InputDeviceType::Touchscreen;
}

bool StrokeRecorder::touchFrame(const TouchFrameEvent &event)
{
    if (!m_isRecordingStroke || event.sender()->type() != InputDeviceType::Touchscreen) {
        return false;
    }

    if (m_blockTouchscreenEventsUntilDeviceNeutral) {
        return true;
    }

    QPointF center;
    const auto validPoints = event.sender()->validTouchPoints();
    for (const auto *point : validPoints) {
        center += point->position;
    }
    center /= validPoints.size();

    if (m_previousTouchscreenTouchCenter.isNull()) {
        m_previousTouchscreenTouchCenter = center;
        return true;
    }

    if (m_previousTouchscreenTouchCenter != center) {
        m_strokePoints.push_back(center - m_previousTouchscreenTouchCenter);
        m_previousTouchscreenTouchCenter = center;
    }
    return true;
}

bool StrokeRecorder::touchUp(const TouchEvent &event)
{
    if (event.sender()->type() != InputDeviceType::Touchscreen) {
        return false;
    }

    if (m_isRecordingStroke) {
        finishStrokeRecording();
        if (!event.sender()->validTouchPoints().empty()) {
            m_blockTouchscreenEventsUntilDeviceNeutral = true;
        }
        return true;
    }

    const auto block = m_blockTouchscreenEventsUntilDeviceNeutral;
    if (event.sender()->validTouchPoints().empty()) {
        m_blockTouchscreenEventsUntilDeviceNeutral = false;
    }
    return block;
}

bool StrokeRecorder::touchpadGestureLifecyclePhase(const TouchpadGestureLifecyclePhaseEvent &event)
{
    if (!m_isRecordingStroke) {
        return false;
    }

    if (event.triggers() == TriggerType::SinglePointMotion
        && (event.phase() == TouchpadGestureLifecyclePhase::End || event.phase() == TouchpadGestureLifecyclePhase::Cancel)) {
        finishStrokeRecording();
    }
    return true;
}

bool StrokeRecorder::touchpadPinch(const TouchpadPinchEvent &event)
{
    return m_isRecordingStroke;
}

bool StrokeRecorder::touchpadSwipe(const MotionEvent &event)
{
    if (!m_isRecordingStroke) {
        return false;
    }

    m_strokePoints.push_back(event.delta().unaccelerated());
    return true;
}

void StrokeRecorder::finishStrokeRecording()
{
    m_isRecordingStroke = false;
    m_strokeCallback(Stroke(m_strokePoints));
    m_strokePoints.clear();
}

}