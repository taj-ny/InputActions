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
#include <libinputactions/input/Keyboard.h>
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

void InputBackend::addCustomDeviceProperties(const QString &name, InputDeviceType type, const InputDeviceProperties &properties)
{
    m_customDeviceProperties[std::make_pair(name, type)] = properties;
}

void InputBackend::applyCustomDeviceProperties(InputDevice *device)
{
    device->properties().apply(customDeviceProperties(device));
}

InputDeviceProperties InputBackend::customDeviceProperties(const InputDevice *device) const
{
    for (const auto &[key, properties] : m_customDeviceProperties) {
        if (key.first == device->name() && key.second == device->type()) {
            return properties;
        }
    }
    return {};
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

std::set<InputDevice *> InputBackend::devices()
{
    return m_devices;
}

void InputBackend::deviceAdded(InputDevice *device)
{
    qCDebug(INPUTACTIONS).noquote().nospace() << "Device added (name: " << device->name() << ")";
    m_devices.insert(device);
    applyCustomDeviceProperties(device);
}

void InputBackend::deviceRemoved(const InputDevice *device)
{
    qCDebug(INPUTACTIONS).noquote().nospace() << "Device removed (name: " << device->name() << ")";
    m_devices.erase(const_cast<InputDevice *>(device));
}

bool InputBackend::isDeviceBlacklisted(const QString &name)
{
    static const std::set<QString> blacklist = {QStringLiteral("inputactions"), QStringLiteral("InputActions Virtual Keyboard"), QStringLiteral("InputActions Virtual Pointer")};
    return blacklist.contains(name);
}

bool InputBackend::handleEvent(const InputEvent &event)
{
    if (!event.sender() || g_sessionLock->sessionLocked()) {
        return false;
    }

    if (event.type() == InputEventType::KeyboardKey) {
        const auto &keyboardEvent = static_cast<const KeyboardKeyEvent &>(event);
        g_keyboard->handleEvent(keyboardEvent);

        if (MODIFIERS.contains(keyboardEvent.nativeKey())) {
            auto modifier = MODIFIERS.at(keyboardEvent.nativeKey());
            if (keyboardEvent.state()) {
                event.sender()->m_modifiers |= modifier;
            } else {
                event.sender()->m_modifiers &= ~modifier;
            }
        }
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