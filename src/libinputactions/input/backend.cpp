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

#include "backend.h"
#include "keyboard.h"
#include "triggers/stroke.h"

namespace libinputactions
{

InputBackend::InputBackend()
{
    m_strokeRecordingTimeoutTimer.setSingleShot(true);
    connect(&m_strokeRecordingTimeoutTimer, &QTimer::timeout, this, &InputBackend::finishStrokeRecording);
}

bool InputBackend::handleEvent(const InputEvent *event)
{
    if (event->type() == InputEventType::KeyboardKey) {
        Keyboard::instance()->handleEvent(static_cast<const KeyboardKeyEvent *>(event));
    }

    for (const auto &handler : m_handlers) {
        if (handler->handleEvent(event)) {
            return true;
        }
    }
    return false;
}

void InputBackend::addEventHandler(std::unique_ptr<InputEventHandler> handler)
{
    m_handlers.push_back(std::move(handler));
}

void InputBackend::clearEventHandlers()
{
    m_handlers.clear();
}

void InputBackend::recordStroke()
{
    m_isRecordingStroke = true;
}

void InputBackend::finishStrokeRecording()
{
    m_isRecordingStroke = false;
    Q_EMIT strokeRecordingFinished(Stroke(m_strokePoints));
    m_strokePoints.clear();
}

void InputBackend::setIgnoreEvents(const bool &value)
{
    m_ignoreEvents = value;
}

InputBackend *InputBackend::instance()
{
    return s_instance.get();
}

void InputBackend::setInstance(std::unique_ptr<InputBackend> instance)
{
    s_instance = std::move(instance);
}

std::unique_ptr<InputBackend> InputBackend::s_instance = std::unique_ptr<InputBackend>(new InputBackend);

}