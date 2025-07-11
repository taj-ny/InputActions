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

#pragma once

#include <QRegularExpression>
#include <QSizeF>
#include <libinputactions/Range.h>
#include <optional>

namespace libinputactions
{

enum class InputDeviceType
{
    Keyboard,
    Mouse,
    Touchpad,
    Unknown
};

class InputDeviceProperties
{
public:
    InputDeviceProperties() = default;

    /**
     * Applies set properties from the other specified properties onto this one.
     */
    void apply(const InputDeviceProperties &other);

    bool multiTouch() const;
    /**
     * Do not set in custom properties unless for testing purposes.
     * @internal
     */
    void setMultiTouch(bool value);

    QSizeF size() const;
    /**
     * Do not set in custom properties unless for testing purposes.
     * @internal
     */
    void setSize(const QSizeF &value);

    bool buttonPad() const;
    /**
     * @param value Overrides whether INPUT_PROP_BUTTONPAD is present.
     */
    void setButtonPad(bool value);

    Range<uint32_t> thumbPressureRange() const;
    void setThumbPressureRange(const Range<uint32_t> &value);

private:
    std::optional<bool> m_multiTouch;
    std::optional<QSizeF> m_size;

    std::optional<bool> m_buttonPad;
    std::optional<Range<uint32_t>> m_thumbPressureRange;
};

class InputDevice
{
public:
    /**
     * @param name Full name of the device.
     * @param sysName Name of the device in /dev/input.
     */
    InputDevice(InputDeviceType type, QString name = {}, QString sysName = {});

    const InputDeviceType &type() const;
    const QString &name() const;
    const QString &sysName() const;
    InputDeviceProperties &properties();
    const InputDeviceProperties &properties() const;

private:
    InputDeviceType m_type;
    QString m_name;
    QString m_sysName;
    InputDeviceProperties m_properties;
};

}