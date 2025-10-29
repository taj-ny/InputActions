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

#include "StandaloneInputEmitter.h"
#include <libinputactions/input/Keyboard.h>

using namespace InputActions;

StandaloneInputEmitter::StandaloneInputEmitter()
{
    if (g_virtualKeyboardUnstableV1->supported()) {
        m_virtualKeyboardUnstableV1Keyboard = g_virtualKeyboardUnstableV1->createKeyboard();
    }
    if (g_wlrVirtualPointerUnstableV1->supported()) {
        m_wlrVirtualPointerUnstableV1Pointer = g_wlrVirtualPointerUnstableV1->createPointer();
    }
}

void StandaloneInputEmitter::keyboardKey(uint32_t key, bool state, const InputDevice *target)
{
    if (m_virtualKeyboardUnstableV1Keyboard) {
        if (MODIFIERS.contains(key)) {
            if (state) {
                m_modifiers |= MODIFIERS.at(key);
            } else {
                m_modifiers &= ~MODIFIERS.at(key);
            }
            m_virtualKeyboardUnstableV1Keyboard->modifiers(m_modifiers);
        }
        m_virtualKeyboardUnstableV1Keyboard->key(key, state);
    } else {
        EvdevInputEmitter::keyboardKey(key, state, target);
    }
}

void StandaloneInputEmitter::mouseButton(uint32_t button, bool state, const InputDevice *target)
{
    if (m_wlrVirtualPointerUnstableV1Pointer) {
        m_wlrVirtualPointerUnstableV1Pointer->button(button, state);
        m_wlrVirtualPointerUnstableV1Pointer->frame();
    } else {
        EvdevInputEmitter::mouseButton(button, state, target);
    }
}

void StandaloneInputEmitter::mouseMoveRelative(const QPointF &delta)
{
    if (m_wlrVirtualPointerUnstableV1Pointer) {
        m_wlrVirtualPointerUnstableV1Pointer->motion(delta);
        m_wlrVirtualPointerUnstableV1Pointer->frame();
    } else {
        EvdevInputEmitter::mouseMoveRelative(delta);
    }
}