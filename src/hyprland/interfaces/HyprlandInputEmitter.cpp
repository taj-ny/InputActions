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

#include "HyprlandInputEmitter.h"

#include <hyprland/src/managers/input/InputManager.hpp>
#include <hyprland/src/managers/KeybindManager.hpp>
#include <hyprland/src/managers/SeatManager.hpp>

#include <hyprland/src/devices/Keyboard.hpp>
#include <hyprland/src/devices/Mouse.hpp>

HyprlandInputEmitter::HyprlandInputEmitter()
    : m_keyboard(SP<IKeyboard>(static_cast<IKeyboard *>(new VirtualKeyboard)))
    , m_pointer(SP<IPointer>(static_cast<IPointer *>(new VirtualPointer)))
{
    m_keyboard->setKeymap({});
    g_pInputManager->m_keyboards.push_back(m_keyboard);
}

HyprlandInputEmitter::~HyprlandInputEmitter()
{
    auto &keyboards = g_pInputManager->m_keyboards;
    keyboards.erase(std::remove(keyboards.begin(), keyboards.end(), m_keyboard), keyboards.end());
}

void HyprlandInputEmitter::keyboardKey(const uint32_t &key, const bool &state)
{
    if (const auto modifier = g_pKeybindManager->keycodeToModifier(key + 8)) {
        auto &modifiers = m_keyboard->m_modifiersState.depressed;
        if (state) {
            modifiers |= modifier;
        } else {
            modifiers &= ~modifier;
        }
        m_keyboard->updateModifiers(modifiers, 0, 0, 0);
        g_pInputManager->onKeyboardMod(m_keyboard);
        m_keyboard->updateXkbStateWithKey(key + 8, state);
    }

    g_pInputManager->onKeyboardKey(IKeyboard::SKeyEvent{
        .keycode = key,
        .state = state ? WL_KEYBOARD_KEY_STATE_PRESSED : WL_KEYBOARD_KEY_STATE_RELEASED
    }, m_keyboard);
}

void HyprlandInputEmitter::mouseButton(const uint32_t &button, const bool &state)
{
    g_pInputManager->onMouseButton(IPointer::SButtonEvent{
        .button = button,
        .state = state ? WL_POINTER_BUTTON_STATE_PRESSED : WL_POINTER_BUTTON_STATE_RELEASED
    });
}

void HyprlandInputEmitter::mouseMoveRelative(const QPointF &pos)
{
    g_pInputManager->onMouseMoved(IPointer::SMotionEvent{
        .delta = Vector2D(pos.x(), pos.y()),
        .device = m_pointer,
    });
}

bool VirtualKeyboard::isVirtual()
{
    return false;
}

SP<Aquamarine::IKeyboard> VirtualKeyboard::aq()
{
    return nullptr;
}

bool VirtualPointer::isVirtual()
{
    return false;
}

SP<Aquamarine::IPointer> VirtualPointer::aq()
{
    return nullptr;
}