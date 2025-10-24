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

#include <QPointF>
#include <QRegularExpression>
#include <QSizeF>
#include <libinputactions/globals.h>
#include <optional>

namespace libinputactions
{

class TouchpadTriggerHandler;

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
     * Only for testing.
     * @internal
     */
    void setMultiTouch(bool value);

    QSizeF size() const;
    /**
     * Only for testing.
     * @internal
     */
    void setSize(const QSizeF &value);

    bool buttonPad() const;
    /**
     * @param value Overrides whether INPUT_PROP_BUTTONPAD is present.
     */
    void setButtonPad(bool value);

    uint32_t fingerPressure() const;
    /**
     * @param value Minimum pressure for a touch point to be considered a finger.
     */
    void setFingerPressure(uint32_t value);

    uint32_t thumbPressure() const;
    /**
     * @param value Minimum pressure for a touch point to be considered a thumb.
     */
    void setThumbPressure(uint32_t value);

    uint32_t palmPressure() const;
    /**
     * @param value Minimum pressure for a touch point to be considered a palm.
     */
    void setPalmPressure(uint32_t value);

    bool lmrTapButtonMap() const;
    /**
     * @param value Whether tapping is mapped to left (1 finger), middle (2) and right (3) buttons.
     */
    void setLmrTapButtonMap(bool value);

private:
    std::optional<bool> m_multiTouch;
    std::optional<QSizeF> m_size;

    std::optional<bool> m_buttonPad;
    std::optional<uint32_t> m_fingerPressure;
    std::optional<uint32_t> m_thumbPressure;
    std::optional<uint32_t> m_palmPressure;

    std::optional<bool> m_lmrTapButtonMap;
};

enum TouchPointType
{
    None,
    Finger,
    Thumb,
    Palm
};

struct TouchPoint
{
    /**
     * Whether this touch point is active and fits within the pressure ranges.
     */
    bool valid{};
    TouchPointType type = TouchPointType::None;

    /**
     * Whether this touch point is active.
     * @internal
     */
    bool active{};

    // These members must not be reset if the point becomes invalid or inactive.
    QPointF initialPosition;
    QPointF position;
    uint32_t pressure{};
    std::chrono::steady_clock::time_point downTimestamp;
};

class InputDevice
{
public:
    /**
     * @param name Full name of the device.
     * @param sysName Name of the device in /dev/input (e.g. event6).
     */
    InputDevice(InputDeviceType type, QString name = {}, QString sysName = {});
    ~InputDevice();

    const InputDeviceType &type() const;
    const QString &name() const;
    const QString &sysName() const;
    InputDeviceProperties &properties();
    const InputDeviceProperties &properties() const;

    /**
     * The size of the vector is equal to the slot count.
     */
    std::vector<TouchPoint> m_touchPoints;
    std::vector<const TouchPoint *> validTouchPoints() const;

    std::unique_ptr<TouchpadTriggerHandler> m_touchpadTriggerHandler;

private:
    InputDeviceType m_type;
    QString m_name;
    QString m_sysName;
    InputDeviceProperties m_properties;
};

}