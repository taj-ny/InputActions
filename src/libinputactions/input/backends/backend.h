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

#include <libinputactions/input/handler.h>

#include <QTimer>

namespace libinputactions
{

class Stroke;

/**
 * Collects input events and forwards them to event handlers.
 */
class InputBackend
{
public:
    virtual ~InputBackend() = default;

    /**
     * Polls and handles events from all devices until there are no events left in the queue.
     */
     virtual void poll();

    /**
     * @param callback Will be called when the stroke has been recorded.
     * @remark Calling this when a stroke is already being recorded will result in the previous callback never being called.
     */
    void recordStroke(const std::function<void(const Stroke &stroke)> &callback);

    void addEventHandler(std::unique_ptr<InputEventHandler> handler);
    void clearEventHandlers();

    void setIgnoreEvents(const bool &value);

    static InputBackend *instance();
    static void setInstance(std::unique_ptr<InputBackend> instance);

protected:
    InputBackend();

    /**
     * @return Whether the event should be blocked.
     */
    bool handleEvent(const InputEvent *event);

    void finishStrokeRecording();

    std::vector<std::unique_ptr<InputEventHandler>> m_handlers;
    bool m_ignoreEvents{};

    bool m_isRecordingStroke = false;
    std::vector<QPointF> m_strokePoints;
    QTimer m_strokeRecordingTimeoutTimer;

private:
    std::function<void(const Stroke &stroke)> m_strokeCallback;

    static std::unique_ptr<InputBackend> s_instance;
};

}