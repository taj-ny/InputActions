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
#include <libinputactions/handlers/TouchpadTriggerHandler.h>

namespace InputActions
{

InputDevice::InputDevice(InputDeviceType type, QString name, QString sysName)
    : m_type(type)
    , m_name(std::move(name))
    , m_sysName(std::move(sysName))
{
}

InputDevice::~InputDevice() = default;

const InputDeviceType &InputDevice::type() const
{
    return m_type;
}

const QString &InputDevice::name() const
{
    return m_name;
}

const QString &InputDevice::sysName() const
{
    return m_sysName;
}

InputDeviceProperties &InputDevice::properties()
{
    return m_properties;
}

const InputDeviceProperties &InputDevice::properties() const
{
    return m_properties;
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

void InputDeviceProperties::setGrab(bool value)
{
    m_grab = value;
}

bool InputDeviceProperties::ignore() const
{
    return m_ignore.value_or(false);
}

void InputDeviceProperties::setIgnore(bool value)
{
    m_ignore = value;
}

bool InputDeviceProperties::handleLibevdevEvents() const
{
    return m_handleLibevdevEvents.value_or(true);
}

void InputDeviceProperties::setHandleLibevdevEvents(bool value)
{
    m_handleLibevdevEvents = value;
}

bool InputDeviceProperties::multiTouch() const
{
    return m_multiTouch.value_or(false);
}

void InputDeviceProperties::setMultiTouch(bool value)
{
    m_multiTouch = value;
}

QSizeF InputDeviceProperties::size() const
{
    return m_size.value_or(QSizeF());
}

void InputDeviceProperties::setSize(const QSizeF &value)
{
    m_size = value;
}

bool InputDeviceProperties::buttonPad() const
{
    return m_buttonPad.value_or(false);
}

void InputDeviceProperties::setButtonPad(bool value)
{
    m_buttonPad = value;
}

uint32_t InputDeviceProperties::fingerPressure() const
{
    return m_fingerPressure.value_or(0);
}

void InputDeviceProperties::setFingerPressure(uint32_t value)
{
    m_fingerPressure = value;
}

uint32_t InputDeviceProperties::thumbPressure() const
{
    return m_thumbPressure.value_or(UINT32_MAX);
}

void InputDeviceProperties::setThumbPressure(uint32_t value)
{
    m_thumbPressure = value;
}

uint32_t InputDeviceProperties::palmPressure() const
{
    return m_palmPressure.value_or(UINT32_MAX);
}

void InputDeviceProperties::setPalmPressure(uint32_t value)
{
    m_palmPressure = value;
}

bool InputDeviceProperties::lmrTapButtonMap() const
{
    return m_lmrTapButtonMap.value_or(false);
}

void InputDeviceProperties::setLmrTapButtonMap(bool value)
{
    m_lmrTapButtonMap = value;
}

}