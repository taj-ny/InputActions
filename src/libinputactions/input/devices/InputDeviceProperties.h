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

#include <QSizeF>
#include <optional>

namespace InputActions
{

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

}