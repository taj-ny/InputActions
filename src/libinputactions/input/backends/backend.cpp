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

#include <libinputactions/input/keyboard.h>
#include <libinputactions/triggers/stroke.h>
#include <libinputactions/variables/manager.h>
#include <libinputactions/variables/variable.h>

#include <QObject>

namespace libinputactions
{

InputBackend::InputBackend()
{
    m_strokeRecordingTimeoutTimer.setSingleShot(true);
    QObject::connect(&m_strokeRecordingTimeoutTimer, &QTimer::timeout, [this] { finishStrokeRecording(); });
}

void InputBackend::addEventHandler(std::unique_ptr<InputEventHandler> handler)
{
    m_handlers.push_back(std::move(handler));
}

void InputBackend::setIgnoreEvents(const bool &value)
{
    m_ignoreEvents = value;
}

void InputBackend::poll()
{
}

std::vector<InputDevice *> InputBackend::devices() const
{
    std::vector<InputDevice *> devices;
    for (auto &device : m_devices) {
        devices.push_back(device.get());
    }
    return devices;
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
    for (auto *device : devices()) {
        removeDevice(device);
    }
    m_customDeviceProperties.clear();
}

void InputBackend::setInstance(std::unique_ptr<InputBackend> instance)
{
    s_instance = std::move(instance);
}

std::unique_ptr<InputBackend> InputBackend::s_instance = std::unique_ptr<InputBackend>(new InputBackend);

void InputBackend::addDevice(std::unique_ptr<InputDevice> device)
{
    auto *raw = device.get();
    m_devices.push_back(std::move(device));
    deviceAdded(raw);

    for (const auto &[name, properties] : m_customDeviceProperties) {
        if (name == raw->name()) {
            raw->properties().apply(properties);
            break;
        }
    }
}

void InputBackend::removeDevice(InputDevice *device)
{
    for (auto it = m_devices.begin(); it != m_devices.end(); it++) {
        auto *raw = it->get();
        if (raw == device) {
            deviceRemoved(raw);
            m_devices.erase(it);
            break;
        }
    }
}

InputDevice *InputBackend::findDevice(const QString &name) const
{
    for (auto &device : m_devices) {
        if (device->name() == name) {
            return device.get();
        }
    }
    return nullptr;
}

void InputBackend::deviceAdded(InputDevice *device)
{
}

void InputBackend::deviceRemoved(const InputDevice *device)
{
}

bool InputBackend::handleEvent(const InputEvent *event)
{
    if (!event->sender()) {
        return false;
    }

    if (event->type() == InputEventType::KeyboardKey) {
        Keyboard::instance()->handleEvent(static_cast<const KeyboardKeyEvent *>(event));
    }
    if (event->sender()->type() != InputDeviceType::Keyboard) {
        VariableManager::instance()->getVariable(BuiltinVariables::DeviceName)->set(event->sender()->name());
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

InputBackend *InputBackend::instance()
{
    return s_instance.get();
}

}