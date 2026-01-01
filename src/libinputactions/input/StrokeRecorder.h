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

#include "InputEventHandler.h"
#include <QObject>
#include <QPointF>
#include <libinputactions/triggers/StrokeTrigger.h>

namespace InputActions
{

class StrokeRecorder
    : public QObject
    , public InputEventHandler
{
public:
    StrokeRecorder();

    /**
     * @param callback Will be called when the stroke has been recorded.
     * @remark Calling this when a stroke is already being recorded will result in the previous callback never being called.
     */
    void recordStroke(const std::function<void(const Stroke &stroke)> &callback);

protected:
    bool pointerAxis(const MotionEvent &event) override;
    bool pointerMotion(const MotionEvent &event) override;

    bool touchpadGestureLifecyclePhase(const TouchpadGestureLifecyclePhaseEvent &event) override;
    bool touchpadPinch(const TouchpadPinchEvent &event) override;
    bool touchpadSwipe(const MotionEvent &event) override;

private:
    void finishStrokeRecording();

    bool m_isRecordingStroke = false;
    std::function<void(const Stroke &stroke)> m_strokeCallback;
    std::vector<QPointF> m_strokePoints;
    QTimer m_strokeRecordingTimeoutTimer;
};

inline std::shared_ptr<StrokeRecorder> g_strokeRecorder;

}