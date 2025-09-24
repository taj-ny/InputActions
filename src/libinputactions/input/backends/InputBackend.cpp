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

#include "InputBackend.h"
#include <QObject>
#include <libinputactions/input/InputEventHandler.h>
#include <libinputactions/interfaces/SessionLock.h>
#include <libinputactions/triggers/StrokeTrigger.h>
#include <libinputactions/variables/Variable.h>
#include <libinputactions/variables/VariableManager.h>

namespace libinputactions
{

InputBackend::InputBackend()
{
    m_strokeRecordingTimeoutTimer.setSingleShot(true);
    QObject::connect(&m_strokeRecordingTimeoutTimer, &QTimer::timeout, [this] {
        finishStrokeRecording();
    });
}

InputBackend::~InputBackend() = default;

void InputBackend::addEventHandler(std::unique_ptr<InputEventHandler> handler)
{
    m_handlers.push_back(std::move(handler));
}

void InputBackend::setIgnoreEvents(bool value)
{
    m_ignoreEvents = value;
}

void InputBackend::poll()
{
}

void InputBackend::addCustomDeviceProperties(const QString &name, const InputDeviceProperties &properties)
{
    m_customDeviceProperties[name] = properties;
}

void InputBackend::initialize()
{
}

void InputBackend::recordStroke(const std::function<void(const Stroke &stroke)> &callback)
{
    m_isRecordingStroke = true;
    m_strokeCallback = callback;
}

void InputBackend::reset()
{
    m_handlers.clear();
    m_customDeviceProperties.clear();
}

void InputBackend::deviceAdded(InputDevice *device)
{
    qCDebug(INPUTACTIONS).noquote().nospace() << "Device added (name: " << device->name() << ")";
    for (const auto &[name, properties] : m_customDeviceProperties) {
        if (name == device->name()) {
            device->properties().apply(properties);
            break;
        }
    }
}

void InputBackend::deviceRemoved(const InputDevice *device)
{
    qCDebug(INPUTACTIONS).noquote().nospace() << "Device removed (name: " << device->name() << ")";
}

bool InputBackend::handleEvent(const InputEvent &event)
{
    if (!event.sender() || g_sessionLock->sessionLocked()) {
        return false;
    }

    if (event.sender()->type() != InputDeviceType::Keyboard) {
        g_variableManager->getVariable(BuiltinVariables::DeviceName)->set(event.sender()->name());
    }

    for (const auto &handler : m_handlers) {
        if (handler->handleEvent(event)) {
            return true;
        }
    }
    return false;
}

void InputBackend::finishStrokeRecording()
{
    m_isRecordingStroke = false;
    m_strokeCallback(Stroke(m_strokePoints));
    m_strokePoints.clear();
}

}