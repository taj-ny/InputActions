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
#include <libinputactions/InputActionsMain.h>
#include <libinputactions/handlers/KeyboardTriggerHandler.h>
#include <libinputactions/handlers/MotionTriggerHandler.h>
#include <libinputactions/handlers/MouseTriggerHandler.h>
#include <libinputactions/handlers/PointerTriggerHandler.h>
#include <libinputactions/handlers/TouchpadTriggerHandler.h>
#include <libinputactions/input/InputDeviceRule.h>
#include <libinputactions/input/InputEventHandler.h>
#include <libinputactions/input/StrokeRecorder.h>
#include <libinputactions/input/events.h>
#include <libinputactions/interfaces/NotificationManager.h>
#include <libinputactions/interfaces/SessionLock.h>
#include <libinputactions/variables/Variable.h>
#include <libinputactions/variables/VariableManager.h>
#include <ranges>

namespace InputActions
{

static const std::chrono::milliseconds EMERGENCY_COMBINATION_HOLD_DURATION{2000L};

InputBackend::InputBackend()
{
    m_emergencyCombinationTimer.setSingleShot(true);
    connect(&m_emergencyCombinationTimer, &QTimer::timeout, this, &InputBackend::onEmergencyCombinationTimerTimeout);
}

InputBackend::~InputBackend() = default;

void InputBackend::setIgnoreEvents(bool value)
{
    m_ignoreEvents = value;
}

InputDeviceProperties InputBackend::deviceProperties(const InputDevice *device) const
{
    InputDeviceProperties properties;
    applyDeviceProperties(device, properties);
    return properties;
}

std::vector<InputDevice *> InputBackend::devices()
{
    return m_devices;
}

InputDevice *InputBackend::firstTouchpad() const
{
    for (auto *device : m_devices) {
        if (device->type() == InputDeviceType::Touchpad) {
            return device;
        }
    }
    return {};
}

Qt::KeyboardModifiers InputBackend::keyboardModifiers() const
{
    Qt::KeyboardModifiers modifiers;
    for (const auto *device : m_devices) {
        modifiers |= device->modifiers();
    }
    return modifiers;
}

void InputBackend::applyDeviceProperties(const InputDevice *device, InputDeviceProperties &properties) const
{
    for (const auto &rule : m_deviceRules | std::ranges::views::reverse) {
        if (!rule.condition()) {
            properties.apply(rule.properties());
            continue;
        }

        VariableManager manager;
        manager.registerLocalVariable<QString>("name").set(device->name());
        manager.registerLocalVariable<InputDeviceTypes>("types").set(device->type());

        ConditionEvaluationArguments arguments;
        arguments.variableManager = &manager;
        if (rule.condition()->satisfied(arguments)) {
            properties.apply(rule.properties());
        }
    }
}

void InputBackend::reset()
{
    m_deviceRules.clear();
    m_keyboardTriggerHandler = {};
    m_mouseTriggerHandler = {};
    m_pointerTriggerHandler = {};
    m_touchpadTriggerHandlerFactory = {};
    m_eventHandlerChain.clear();
}

void InputBackend::addDevice(InputDevice *device)
{
    qCDebug(INPUTACTIONS).noquote().nospace() << "Device added (name: " << device->name() << ")";
    applyDeviceProperties(device, device->properties());

    if (device->type() == InputDeviceType::Touchpad && m_touchpadTriggerHandlerFactory) {
        device->setTouchpadTriggerHandler(m_touchpadTriggerHandlerFactory(device));
    }

    m_devices.push_back(device);
    createEventHandlerChain();
}

void InputBackend::removeDevice(const InputDevice *device)
{
    qCDebug(INPUTACTIONS).noquote().nospace() << "Device removed (name: " << device->name() << ")";
    std::erase(m_devices, device);
    createEventHandlerChain();
}

void InputBackend::createEventHandlerChain()
{
    const auto pushToChain = [this](const auto &handler) {
        if (handler) {
            m_eventHandlerChain.push_back(handler.get());
        }
    };

    m_eventHandlerChain.clear();
    m_eventHandlerChain.push_back(g_strokeRecorder.get());
    pushToChain(m_keyboardTriggerHandler);
    pushToChain(m_mouseTriggerHandler);
    for (const auto &device : m_devices) {
        if (const auto &touchpadTriggerHandler = device->touchpadTriggerHandler()) {
            m_eventHandlerChain.push_back(touchpadTriggerHandler);
        }
    }
    pushToChain(m_pointerTriggerHandler);
}

bool InputBackend::handleEvent(const InputEvent &event)
{
    if (!event.sender() || event.sender()->properties().ignore()) {
        return false;
    }

    if (event.type() == InputEventType::KeyboardKey && !m_emergencyCombination.empty()) {
        m_emergencyCombinationTimer.stop();
        if (event.sender()->keys() == m_emergencyCombination) {
            m_emergencyCombinationTimer.start(EMERGENCY_COMBINATION_HOLD_DURATION);
        }
    }

    if (g_sessionLock->sessionLocked()) {
        return false;
    }

    if (event.sender()->type() != InputDeviceType::Keyboard) {
        g_variableManager->getVariable(BuiltinVariables::DeviceName)->set(event.sender()->name());
    }

    for (auto *handler : m_eventHandlerChain) {
        if (handler->handleEvent(event)) {
            return true;
        }
    }
    return false;
}

void InputBackend::onEmergencyCombinationTimerTimeout()
{
    g_notificationManager->sendNotification("Emergency combination", "Emergency combination triggered, suspending may take up to a few seconds");
    g_inputActions->suspend();
}

void InputBackend::setDeviceRules(std::vector<InputDeviceRule> rules)
{
    m_deviceRules = std::move(rules);
}

void InputBackend::setKeyboardTriggerHandler(std::unique_ptr<KeyboardTriggerHandler> value)
{
    m_keyboardTriggerHandler = std::move(value);
}

void InputBackend::setMouseTriggerHandler(std::unique_ptr<MouseTriggerHandler> value)
{
    m_mouseTriggerHandler = std::move(value);
}

void InputBackend::setPointerTriggerHandler(std::unique_ptr<PointerTriggerHandler> value)
{
    m_pointerTriggerHandler = std::move(value);
}

void InputBackend::setTouchpadTriggerHandlerFactory(std::function<std::unique_ptr<TouchpadTriggerHandler>(InputDevice *device)> value)
{
    m_touchpadTriggerHandlerFactory = std::move(value);
}

}