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

#include <libinputactions/input/backends/InputBackend.h>
#include <libinputactions/input/Keyboard.h>

using namespace libinputactions;

StandaloneInputEmitter::StandaloneInputEmitter()
{
    if (VirtualKeyboardUnstableV1::instance()->supported()) {
        m_virtualKeyboardUnstableV1Keyboard = VirtualKeyboardUnstableV1::instance()->createKeyboard();
    }
    if (WlrVirtualPointerUnstableV1::instance()->supported()) {
        m_wlrVirtualPointerUnstableV1Pointer = WlrVirtualPointerUnstableV1::instance()->createPointer();
    }
}

void StandaloneInputEmitter::keyboardKey(const uint32_t &key, const bool &state)
{
    InputBackend::instance()->setIgnoreEvents(true);
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
    }
    InputBackend::instance()->setIgnoreEvents(false);
}

void StandaloneInputEmitter::mouseButton(const uint32_t &button, const bool &state)
{
    InputBackend::instance()->setIgnoreEvents(true);
    if (m_wlrVirtualPointerUnstableV1Pointer) {
        m_wlrVirtualPointerUnstableV1Pointer->button(button, state);
        m_wlrVirtualPointerUnstableV1Pointer->frame();
    }
    InputBackend::instance()->setIgnoreEvents(false);
}

void StandaloneInputEmitter::mouseMoveRelative(const QPointF &delta)
{
    InputBackend::instance()->setIgnoreEvents(true);
    if (m_wlrVirtualPointerUnstableV1Pointer) {
        m_wlrVirtualPointerUnstableV1Pointer->motion(delta);
        m_wlrVirtualPointerUnstableV1Pointer->frame();
    }
    InputBackend::instance()->setIgnoreEvents(false);
}