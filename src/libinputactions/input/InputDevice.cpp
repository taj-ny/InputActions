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

#include "InputDevice.h"
#include "backends/InputBackend.h"
#include <libinputactions/handlers/TouchpadTriggerHandler.h>
#include <libinputactions/handlers/TouchscreenTriggerHandler.h>

namespace InputActions
{

InputDevice::InputDevice(InputDeviceType type, QString name, QString sysName)
    : m_type(type)
    , m_name(std::move(name))
    , m_sysName(std::move(sysName))
{
    m_touchscreenTapTimer.setSingleShot(true);
    m_touchscreenTapTimer.setTimerType(Qt::PreciseTimer);
    connect(&m_touchscreenTapTimer, &QTimer::timeout, this, &InputDevice::onTouchscreenTapTimerTimeout);
}

InputDevice::~InputDevice() = default;

void InputDevice::simulateTouchscreenTap(const std::vector<QPointF> &points)
{
    if (m_touchscreenTapTimer.isActive() || !validTouchPoints().empty()) {
        return;
    }

    simulateTouchscreenTapDown(points);
    m_touchscreenTapPoints = points;
    m_touchscreenTapTimer.start(10);
}

void InputDevice::onTouchscreenTapTimerTimeout()
{
    simulateTouchscreenTapUp(m_touchscreenTapPoints);
}

Qt::KeyboardModifiers InputDevice::modifiers() const
{
    Qt::KeyboardModifiers modifiers;
    for (const auto &[key, modifier] : KEYBOARD_MODIFIERS) {
        if (m_keys.contains(key)) {
            modifiers |= modifier;
        }
    }
    return modifiers;
}

void InputDevice::setKeyState(uint32_t key, bool state)
{
    if (state) {
        m_keys.insert(key);
    } else {
        m_keys.erase(key);
    }
}

std::vector<const TouchPoint *> InputDevice::validTouchPoints() const
{
    std::vector<const TouchPoint *> result;
    for (auto &point : m_touchPoints) {
        if (point.valid) {
            result.push_back(&point);
        }
    }
    std::ranges::sort(result, [](const auto *a, const auto *b) {
        return a->downTimestamp < b->downTimestamp;
    });
    return result;
}

void InputDevice::setTouchpadTriggerHandler(std::unique_ptr<TouchpadTriggerHandler> value)
{
    m_touchpadTriggerHandler = std::move(value);
}

void InputDevice::setTouchscreenTriggerHandler(std::unique_ptr<TouchscreenTriggerHandler> value)
{
    m_touchscreenTriggerHandler = std::move(value);
}

void InputDeviceProperties::apply(const InputDeviceProperties &other)
{
    static const auto apply = [](auto &thisOpt, auto &otherOpt) {
        if (otherOpt) {
            thisOpt = otherOpt;
        }
    };

    apply(m_grab, other.m_grab);
    apply(m_ignore, other.m_ignore);
    apply(m_handleLibevdevEvents, other.m_handleLibevdevEvents);
    apply(m_multiTouch, other.m_multiTouch);
    apply(m_size, other.m_size);
    apply(m_buttonPad, other.m_buttonPad);
    apply(m_fingerPressure, other.m_fingerPressure);
    apply(m_thumbPressure, other.m_thumbPressure);
    apply(m_palmPressure, other.m_palmPressure);
    apply(m_lmrTapButtonMap, other.m_lmrTapButtonMap);
}

bool InputDeviceProperties::grab() const
{
    return m_grab.value_or(false);
}

bool InputDeviceProperties::ignore() const
{
    return m_ignore.value_or(false);
}

bool InputDeviceProperties::handleLibevdevEvents() const
{
    return m_handleLibevdevEvents.value_or(true);
}

bool InputDeviceProperties::multiTouch() const
{
    return m_multiTouch.value_or(false);
}

QSizeF InputDeviceProperties::size() const
{
    return m_size.value_or(QSizeF());
}

bool InputDeviceProperties::buttonPad() const
{
    return m_buttonPad.value_or(false);
}

uint32_t InputDeviceProperties::fingerPressure() const
{
    return m_fingerPressure.value_or(0);
}

uint32_t InputDeviceProperties::thumbPressure() const
{
    return m_thumbPressure.value_or(UINT32_MAX);
}

uint32_t InputDeviceProperties::palmPressure() const
{
    return m_palmPressure.value_or(UINT32_MAX);
}

bool InputDeviceProperties::lmrTapButtonMap() const
{
    return m_lmrTapButtonMap.value_or(false);
}

}