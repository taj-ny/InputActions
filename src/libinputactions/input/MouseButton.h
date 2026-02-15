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

#include <QString>
#include <QVector>
#include <linux/input-event-codes.h>
#include <optional>
#include <unordered_map>

// https://invent.kde.org/plasma/kwin/-/blob/cc4d99ae/src/mousebuttons.cpp#L14
inline const std::unordered_map<QString, uint32_t> MOUSE_BUTTONS = {
    {"LEFT", BTN_LEFT},
    {"MIDDLE", BTN_MIDDLE},
    {"RIGHT", BTN_RIGHT},

    {"BACK", BTN_SIDE},
    {"FORWARD", BTN_EXTRA},
    {"TASK", BTN_FORWARD},
    {"SIDE", BTN_BACK},
    {"EXTRA", BTN_TASK},

    // Backwards compatibility (trigger mouse button list supported these)
    {"EXTRA1", BTN_SIDE},
    {"EXTRA2", BTN_EXTRA},
    {"EXTRA3", BTN_FORWARD},
    {"EXTRA4", BTN_BACK},
    {"EXTRA5", BTN_TASK},

    {"EXTRA6", 0x118},
    {"EXTRA7", 0x119},
    {"EXTRA8", 0x11a},
    {"EXTRA9", 0x11b},
    {"EXTRA10", 0x11c},
    {"EXTRA11", 0x11d},
    {"EXTRA12", 0x11e},
    {"EXTRA13", 0x11f},
};

namespace InputActions
{

class MouseButton
{
public:
    MouseButton(uint32_t scanCode = 0);

    static std::optional<MouseButton> fromString(QString s);

    uint32_t scanCode() const { return m_scanCode; }

    auto operator<=>(const MouseButton &) const = default;

    operator bool() const { return m_scanCode; }

private:
    uint32_t m_scanCode;
};

}