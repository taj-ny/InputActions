/*
    Input Actions - Input handler that executes user-defined actions
    Copyright (C) 2024-2026 Marcin Woźniak

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

#include "InputDeviceProperties.h"
#include <QMetaProperty>
#include <QStringList>
#include <libinputactions/utils/QVariantUtils.h>

namespace InputActions
{

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

QString InputDeviceProperties::toString() const
{
    QStringList result;

    for (auto i = 0; i < staticMetaObject.propertyCount(); ++i) {
        const auto property = staticMetaObject.property(i);
        result += QString("%1: %2").arg(property.name(), QVariantUtils::toString(property.readOnGadget(this)));
    }

    return result.join('\n');
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