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
#include "conditions/VariableCondition.h"
#include <QObject>
#include <libinputactions/conditions/Condition.h>
#include <libinputactions/handlers/KeyboardTriggerHandler.h>
#include <libinputactions/handlers/MotionTriggerHandler.h>
#include <libinputactions/handlers/MouseTriggerHandler.h>
#include <libinputactions/handlers/PointerTriggerHandler.h>
#include <libinputactions/handlers/TouchpadTriggerHandler.h>
#include <libinputactions/input/InputDeviceRule.h>
#include <libinputactions/input/InputEventHandler.h>
#include <libinputactions/input/Keyboard.h>
#include <libinputactions/input/events.h>
#include <libinputactions/interfaces/SessionLock.h>
#include <libinputactions/triggers/StrokeTrigger.h>
#include <libinputactions/variables/Variable.h>
#include <libinputactions/variables/VariableManager.h>
#include <ranges>

namespace InputActions
{

InputBackend::InputBackend()
{
    m_strokeRecordingTimeoutTimer.setSingleShot(true);
    QObject::connect(&m_strokeRecordingTimeoutTimer, &QTimer::timeout, [this] {
        finishStrokeRecording();
    });
}

InputBackend::~InputBackend() = default;

void InputBackend::setIgnoreEvents(bool value)
{
    m_ignoreEvents = value;
}

void InputBackend::poll() {}

void InputBackend::initialize()
{
    InputDeviceRule ignoreOwnDevicesRule;
    ignoreOwnDevicesRule.m_condition = std::make_shared<VariableCondition>("name", std::vector<Value<std::any>>{Value(QStringLiteral("inputactions")), Value(QStringLiteral("InputActions Virtual Keyboard")), Value(QStringLiteral("InputActions Virtual Pointer"))}, ComparisonOperator::OneOf);
    ignoreOwnDevicesRule.m_properties.setIgnore(true);
    m_deviceRules.push_back(std::move(ignoreOwnDevicesRule));
}

void InputBackend::recordStroke(const std::function<void(const Stroke &stroke)> &callback)
{
    m_isRecordingStroke = true;
    m_strokeCallback = callback;
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

void InputBackend::applyDeviceProperties(const InputDevice *device, InputDeviceProperties &properties) const
{
    for (const auto &rule : m_deviceRules | std::ranges::views::reverse) {
        if (!rule.m_condition) {
            properties.apply(rule.m_properties);
            continue;
        }

        VariableManager manager;
        manager.registerLocalVariable<QString>("name").set(device->name());
        manager.registerLocalVariable<InputDeviceTypes>("types").set(device->type());

        ConditionEvaluationArguments arguments;
        arguments.variableManager = &manager;
        if (rule.m_condition->satisfied(arguments)) {
            properties.apply(rule.m_properties);
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

void InputBackend::deviceAdded(InputDevice *device)
{
    qCDebug(INPUTACTIONS).noquote().nospace() << "Device added (name: " << device->name() << ")";
    applyDeviceProperties(device, device->properties());

    if (device->type() == InputDeviceType::Touchpad && m_touchpadTriggerHandlerFactory) {
        device->m_touchpadTriggerHandler = m_touchpadTriggerHandlerFactory(device);
    }

    m_devices.push_back(device);
    createEventHandlerChain();
}

void InputBackend::deviceRemoved(const InputDevice *device)
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
    pushToChain(m_keyboardTriggerHandler);
    pushToChain(m_mouseTriggerHandler);
    for (const auto &device : m_devices) {
        if (const auto &touchpadTriggerHandler = device->m_touchpadTriggerHandler) {
            m_eventHandlerChain.push_back(touchpadTriggerHandler.get());
        }
    }
    pushToChain(m_pointerTriggerHandler);
}

bool InputBackend::handleEvent(const InputEvent &event)
{
    if (!event.sender() || g_sessionLock->sessionLocked() || event.sender()->properties().ignore()) {
        return false;
    }

    if (event.type() == InputEventType::KeyboardKey) {
        const auto &keyboardEvent = static_cast<const KeyboardKeyEvent &>(event);
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

    for (auto *handler : m_eventHandlerChain) {
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