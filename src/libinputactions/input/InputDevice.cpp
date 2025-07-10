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

namespace libinputactions
{

InputDevice::InputDevice(InputDeviceType type, QString name, QString sysName)
    : m_type(type)
    , m_name(std::move(name))
    , m_sysName(std::move(sysName))
{
}

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

void InputDeviceProperties::apply(const InputDeviceProperties &other)
{
    if (other.m_multiTouch) {
        m_multiTouch = other.m_multiTouch;
    }
    if (other.m_size) {
        m_size = other.m_size;
    }
    if (other.m_buttonPad) {
        m_buttonPad = other.m_buttonPad;
    }
    if (other.m_thumbPressureRange) {
        m_thumbPressureRange = other.m_thumbPressureRange;
    }
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

Range<uint32_t> InputDeviceProperties::thumbPressureRange() const
{
    return m_thumbPressureRange.value_or(Range<uint32_t>(-1, -1));
}

void InputDeviceProperties::setThumbPressureRange(const Range<uint32_t> &value)
{
    m_thumbPressureRange = value;
}

}