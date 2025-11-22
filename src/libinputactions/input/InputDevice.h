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
#include <linux/input-event-codes.h>
#include <optional>
#include <unordered_set>

namespace InputActions
{

class TouchpadTriggerHandler;

static const std::map<uint32_t, Qt::KeyboardModifier> KEYBOARD_MODIFIERS{
    {KEY_LEFTALT, Qt::KeyboardModifier::AltModifier},
    {KEY_LEFTCTRL, Qt::KeyboardModifier::ControlModifier},
    {KEY_LEFTMETA, Qt::KeyboardModifier::MetaModifier},
    {KEY_LEFTSHIFT, Qt::KeyboardModifier::ShiftModifier},
    {KEY_RIGHTALT, Qt::KeyboardModifier::AltModifier},
    {KEY_RIGHTCTRL, Qt::KeyboardModifier::ControlModifier},
    {KEY_RIGHTMETA, Qt::KeyboardModifier::MetaModifier},
    {KEY_RIGHTSHIFT, Qt::KeyboardModifier::ShiftModifier},
};

class InputDeviceProperties
{
public:
    InputDeviceProperties() = default;

    /**
     * Applies set properties from the other specified properties onto this one.
     */
    void apply(const InputDeviceProperties &other);

    /**
     * @returns Whether the device should be grabbed (standalone only).
     */
    bool grab() const;
    /**
     * @see grab
     */
    void setGrab(bool value);

    /**
     * @returns Whether the device should be ignored completely.
     */
    bool ignore() const;
    /**
     * @see ignore
     */
    void setIgnore(bool value);

    /**
     * @returns Whether to process libevdev events if available.
     */
    bool handleLibevdevEvents() const;
    /**
     * @see handleLibevdevEvents
     */
    void setHandleLibevdevEvents(bool value);

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
    std::optional<bool> m_grab;
    std::optional<bool> m_ignore;
    std::optional<bool> m_handleLibevdevEvents;

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

    /**
     * Current keyboard modifiers, derived from pressed keyboard keys.
     */
    Qt::KeyboardModifiers modifiers() const;

    /**
     * Currently pressed keyboard keys.
     */
    const std::unordered_set<uint32_t> &keys() const;
    void setKeyState(uint32_t key, bool state);

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

    std::unordered_set<uint32_t> m_keys;
};

}