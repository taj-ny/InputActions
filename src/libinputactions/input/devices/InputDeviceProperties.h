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

#pragma once

#include <QMetaObject>
#include <QSizeF>
#include <chrono>
#include <optional>

namespace InputActions
{

class InputDeviceProperties
{
    Q_GADGET

    Q_PROPERTY(uint32_t fingerPressure READ fingerPressure)
    Q_PROPERTY(bool grab READ grab)
    Q_PROPERTY(bool handleLibevdevEvents READ handleLibevdevEvents)
    Q_PROPERTY(bool ignore READ ignore)
    Q_PROPERTY(bool multiTouch READ multiTouch)
    Q_PROPERTY(uint32_t palmPressure READ palmPressure)
    Q_PROPERTY(QSizeF size READ size)
    Q_PROPERTY(uint32_t thumbPressure READ thumbPressure)

    Q_PROPERTY(std::chrono::milliseconds mouseMotionTimeout READ mouseMotionTimeout)
    Q_PROPERTY(std::chrono::milliseconds mousePressTimeout READ mousePressTimeout)
    Q_PROPERTY(bool mouseUnblockButtonsOnTimeout READ mouseUnblockButtonsOnTimeout)

    Q_PROPERTY(bool touchpadButtonPad READ touchpadButtonPad)
    Q_PROPERTY(std::chrono::milliseconds touchpadClickTimeout READ touchpadClickTimeout)
    Q_PROPERTY(bool touchpadLmrTapButtonMap READ touchpadLmrTapButtonMap)

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
     */
    void setMultiTouch(bool value) { m_multiTouch = value; }

    bool hasSize() const;
    QSizeF size() const;
    /**
     * Only for testing.
     */
    void setSize(const QSizeF &value) { m_size = value; }
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
     * The amount of time in the handler will wait for motion to be performed (wheel is considered motion as well) before attempting to activate press triggers.
     * For pointer motion there is a small threshold to prevent accidental activations.
     */
    std::chrono::milliseconds mouseMotionTimeout() const;
    void setMouseMotionTimeout(std::chrono::milliseconds value) { m_mouseMotionTimeout = value; }

    /**
     * The amount of time the handler will wait for all mouse buttons to be pressed before activating press triggers.
     */
    std::chrono::milliseconds mousePressTimeout() const;
    void setMousePressTimeout(std::chrono::milliseconds value) { m_mousePressTimeout = value; }

    /**
     * Whether blocked mouse buttons should be pressed immediately on timeout. If false, they will be pressed and instantly released on button release.
     */
    bool mouseUnblockButtonsOnTimeout() const;
    void setMouseUnblockButtonsOnTimeout(bool value) { m_mouseUnblockButtonsOnTimeout = value; }

    /**
     * Whether INPUT_PROP_BUTTONPAD is present.
     */
    bool touchpadButtonPad() const;
    void setTouchpadButtonPad(bool value) { m_touchpadButtonPad = value; }

    /**
     * The time for the user to perform a click once a press gesture had been detected by libinput. If the click is not performed, the press trigger is
     * activated.
     */
    std::chrono::milliseconds touchpadClickTimeout() const;
    void setTouchpadClickTimeout(std::chrono::milliseconds value) { m_touchpadClickTimeout = value; }

    /**
     * Whether tapping is mapped to left (1 finger), middle (2) and right (3) buttons.
     */
    bool touchpadLmrTapButtonMap() const;
    void setTouchpadLmrTapButtonMap(bool value) { m_touchpadLmrTapButtonMap = value; }

    QString toString() const;

private:
    std::optional<bool> m_grab;
    std::optional<bool> m_ignore;
    std::optional<bool> m_handleLibevdevEvents;

    std::optional<bool> m_multiTouch;
    std::optional<QSizeF> m_size;

    std::optional<uint32_t> m_fingerPressure;
    std::optional<uint32_t> m_thumbPressure;
    std::optional<uint32_t> m_palmPressure;

    std::optional<std::chrono::milliseconds> m_mouseMotionTimeout;
    std::optional<std::chrono::milliseconds> m_mousePressTimeout;
    std::optional<bool> m_mouseUnblockButtonsOnTimeout;

    std::optional<bool> m_touchpadButtonPad;
    std::optional<std::chrono::milliseconds> m_touchpadClickTimeout;
    std::optional<bool> m_touchpadLmrTapButtonMap;

    friend class TestDeviceRuleNodeParser;
};

}