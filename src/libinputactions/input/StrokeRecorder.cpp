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

bool StrokeRecorder::acceptsEvent(const InputEvent &event)
{
    return true;
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