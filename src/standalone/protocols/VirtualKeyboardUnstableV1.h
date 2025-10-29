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

#include "WaylandProtocol.h"
#include "virtual-keyboard-unstable-v1.h"
#include <xkbcommon/xkbcommon.h>

class VirtualKeyboardUnstableV1Keyboard
{
public:
    VirtualKeyboardUnstableV1Keyboard(zwp_virtual_keyboard_manager_v1 *manager);
    ~VirtualKeyboardUnstableV1Keyboard();

    void key(uint32_t key, bool state);
    void modifiers(Qt::KeyboardModifiers modifiers);

    const bool &valid() const;

private:
    zwp_virtual_keyboard_v1 *m_keyboard{};
    xkb_keymap *m_keymap{};
    bool m_valid{};
};

class VirtualKeyboardUnstableV1 : public WaylandProtocol
{
public:
    VirtualKeyboardUnstableV1();
    ~VirtualKeyboardUnstableV1() override;

    /**
     * @returns The keyboard interface, or nullptr if creation failed.
     */
    std::unique_ptr<VirtualKeyboardUnstableV1Keyboard> createKeyboard();

protected:
    void bind(wl_registry *registry, uint32_t name, uint32_t version) override;

private:
    zwp_virtual_keyboard_manager_v1 *m_manager{};
};

inline std::unique_ptr<VirtualKeyboardUnstableV1> g_virtualKeyboardUnstableV1;