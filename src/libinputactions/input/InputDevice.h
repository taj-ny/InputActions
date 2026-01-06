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
#include <QTimer>
#include <libinputactions/globals.h>
#include <linux/input-event-codes.h>
#include <optional>
#include <unordered_set>

namespace InputActions
{

class TouchpadTriggerHandler;
class TouchscreenTriggerHandler;

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
     * Whether the device should be grabbed (standalone only).
     */
    bool grab() const;
    void setGrab(bool value) { m_grab = value; }

    /**
     * Whether the device should be ignored completely.
     */
    bool ignore() const;
    void setIgnore(bool value) { m_ignore = value; }

    /**
     * Whether to process libevdev events if available.
     */
    bool handleLibevdevEvents() const;
    void setHandleLibevdevEvents(bool value) { m_handleLibevdevEvents = value; }

    bool multiTouch() const;
    /**
     * Only for testing.
     * @internal
     */
    void setMultiTouch(bool value) { m_multiTouch = value; }

    QSizeF size() const;
    /**
     * Only for testing.
     * @internal
     */
    void setSize(const QSizeF &value) { m_size = value; }

    /**
     * Whether INPUT_PROP_BUTTONPAD is present.
     */
    bool buttonPad() const;
    void setButtonPad(bool value) { m_buttonPad = value; }

    /**
     * Minimum pressure for a touch point to be considered a finger.
     */
    uint32_t fingerPressure() const;
    void setFingerPressure(uint32_t value) { m_fingerPressure = value; }

    /**
     * Minimum pressure for a touch point to be considered a thumb.
     */
    uint32_t thumbPressure() const;
    void setThumbPressure(uint32_t value) { m_thumbPressure = value; }

    /**
     * Minimum pressure for a touch point to be considered a palm.
     */
    uint32_t palmPressure() const;
    void setPalmPressure(uint32_t value) { m_palmPressure = value; }

    /**
     * Whether tapping is mapped to left (1 finger), middle (2) and right (3) buttons.
     */
    bool lmrTapButtonMap() const;
    void setLmrTapButtonMap(bool value) { m_lmrTapButtonMap = value; }

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
     * May be unused.
     */
    int32_t id{};

    /**
     * Whether this touch point is active.
     * @internal
     */
    bool active{};

    // These members must not be reset if the point becomes invalid or inactive.
    /**
     * Unaltered current position, as provided by the compositor/evdev. Only used for touchscreens.
     */
    QPointF unalteredPosition;
    /**
     * Unaltered current position, as provided by the compositor/evdev. Only used for touchscreens.
     */
    QPointF unalteredInitialPosition;

    QPointF position;
    QPointF initialPosition;
    uint32_t pressure{};
    std::chrono::steady_clock::time_point downTimestamp;
};

/**
 * Each device has two states:
 *   - physical - actual state of the device,
 *   - virtual - the state of the device as seen by another entity that is processing events - the compositor and its libinput instance, an external libinput
 *    instance, evtest, etc. InputActions manipulates this state in various ways for the purposes of event filtering.
 */
class InputDevice : public QObject
{
    Q_OBJECT

public:
    /**
     * @param name Full name of the device.
     * @param sysName Name of the device in /dev/input (e.g. event6).
     */
    InputDevice(InputDeviceType type, QString name = {}, QString sysName = {});
    ~InputDevice() override;

    /**
     * Sets the device's virtual state into a neutral one. In the standalone implementation, the device must be grabbed, otherwise the call will be ignored.
     *
     * This operation is currently only used for touchscreens and touchpads (standalone only).
     */
    virtual void resetVirtualDeviceState() {}
    /**
     * Restores the device's virtual state to the physical one. In the standalone implementation, the device must be grabbed, otherwise the call will be
     * ignored.
     *
     * This operation is currently only used for touchscreens and touchpads (standalone only).
     *
     * The touchscreen restore sequence must include the following elements:
     *   - Touch down - at **initial positions**, not current
     *   - Touch frame
     *   - Touch motion - from initial positions to current positions
     *   - Touch frame
     * More elements may be added by the implementation if necessary.
     */
    virtual void restoreVirtualDeviceState() {}

    /**
     * @param points Unaltered points from events provided by the backend.
     * @see TouchPoint::unalteredPosition
     */
    void simulateTouchscreenTap(const std::vector<QPointF> &points);

    /**
     * Current keyboard modifiers, derived from pressed keyboard keys.
     */
    Qt::KeyboardModifiers modifiers() const;

    /**
     * Currently pressed keyboard keys.
     */
    const std::unordered_set<uint32_t> &keys() const { return m_keys; }
    void setKeyState(uint32_t key, bool state);

    const InputDeviceType &type() const { return m_type; }
    const QString &name() const { return m_name; }
    const QString &sysName() const { return m_sysName; }
    InputDeviceProperties &properties() { return m_properties; }
    const InputDeviceProperties &properties() const { return m_properties; }

    /**
     * The size of the vector is equal to the slot count.
     */
    const std::vector<TouchPoint> &touchPoints() const { return m_touchPoints; }
    std::vector<TouchPoint> &touchPoints() { return m_touchPoints; }

    std::vector<const TouchPoint *> validTouchPoints() const;
    void setTouchPoints(std::vector<TouchPoint> value) { m_touchPoints = std::move(value); }

    TouchpadTriggerHandler *touchpadTriggerHandler() const { return m_touchpadTriggerHandler.get(); }
    void setTouchpadTriggerHandler(std::unique_ptr<TouchpadTriggerHandler> value);

    TouchscreenTriggerHandler *touchscreenTriggerHandler() const { return m_touchscreenTriggerHandler.get(); }
    void setTouchscreenTriggerHandler(std::unique_ptr<TouchscreenTriggerHandler> value);

protected:
    /**
     * Must generate touch down events and a touch frame event for the specified points.
     */
    virtual void simulateTouchscreenTapDown(const std::vector<QPointF> &points) {}
    /**
     * Must generate touch up events and a touch frame event for the specified points.
     */
    virtual void simulateTouchscreenTapUp(const std::vector<QPointF> &points) {}

private slots:
    void onTouchscreenTapTimerTimeout();

private:
    InputDeviceType m_type;
    QString m_name;
    QString m_sysName;
    InputDeviceProperties m_properties;
    std::unordered_set<uint32_t> m_keys;
    std::vector<TouchPoint> m_touchPoints;
    std::unique_ptr<TouchpadTriggerHandler> m_touchpadTriggerHandler;

    QTimer m_touchscreenTapTimer;
    std::vector<QPointF> m_touchscreenTapPoints;
    std::unique_ptr<TouchscreenTriggerHandler> m_touchscreenTriggerHandler;
};

}