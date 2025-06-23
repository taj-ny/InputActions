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

#include <libinputactions/input/backends/InputBackend.h>
#include <libinputactions/input/Keyboard.h>

#include <hyprland/src/managers/input/InputManager.hpp>
#include <hyprland/src/managers/KeybindManager.hpp>
#include <hyprland/src/managers/SeatManager.hpp>
#include <hyprland/src/protocols/PointerGestures.hpp>
#undef HANDLE

HyprlandInputEmitter::HyprlandInputEmitter()
    : m_keyboard(makeShared<VirtualKeyboard>())
    , m_pointer(makeShared<VirtualPointer>())
{
    g_pInputManager->newKeyboard(m_keyboard);
}

HyprlandInputEmitter::~HyprlandInputEmitter()
{
    m_keyboard->events.destroy.emit();
}

void HyprlandInputEmitter::keyboardClearModifiers()
{
    libinputactions::InputBackend::instance()->setIgnoreEvents(true);
    m_modifiers = 0;
    const auto modifiers = libinputactions::Keyboard::instance()->modifiers();
    for (auto &keyboard : g_pInputManager->m_keyboards) {
        if (auto aqKeyboard = keyboard->aq()) {
            for (const auto &[key, modifier] : libinputactions::MODIFIERS) {
                if (modifiers & modifier) {
                    aqKeyboard->events.key.emit(Aquamarine::IKeyboard::SKeyEvent{
                        .key = key,
                        .pressed = false
                    });
                }
            }
            aqKeyboard->events.modifiers.emit(Aquamarine::IKeyboard::SModifiersEvent{
                .depressed = 0
            });
        }
    }
    libinputactions::InputBackend::instance()->setIgnoreEvents(false);
}

void HyprlandInputEmitter::keyboardKey(const uint32_t &key, const bool &state)
{
    libinputactions::InputBackend::instance()->setIgnoreEvents(true);
    if (const auto modifier = g_pKeybindManager->keycodeToModifier(key + 8)) {
        if (state) {
            m_modifiers |= modifier;
        } else {
            m_modifiers &= ~modifier;
        }
        m_keyboard->events.modifiers.emit(Aquamarine::IKeyboard::SModifiersEvent{
            .depressed = m_modifiers
        });
    }

    m_keyboard->events.key.emit(Aquamarine::IKeyboard::SKeyEvent{
        .key = key,
        .pressed = state
    });
    libinputactions::InputBackend::instance()->setIgnoreEvents(false);
}

void HyprlandInputEmitter::mouseButton(const uint32_t &button, const bool &state)
{
    libinputactions::InputBackend::instance()->setIgnoreEvents(true);
    g_pInputManager->onMouseButton(IPointer::SButtonEvent{
        .button = button,
        .state = state ? WL_POINTER_BUTTON_STATE_PRESSED : WL_POINTER_BUTTON_STATE_RELEASED
    });
    libinputactions::InputBackend::instance()->setIgnoreEvents(false);
}

void HyprlandInputEmitter::mouseMoveRelative(const QPointF &pos)
{
    libinputactions::InputBackend::instance()->setIgnoreEvents(true);
    g_pInputManager->onMouseMoved(IPointer::SMotionEvent{
        .delta = Vector2D(pos.x(), pos.y()),
        .device = m_pointer,
    });
    libinputactions::InputBackend::instance()->setIgnoreEvents(false);
}

void HyprlandInputEmitter::touchpadPinchBegin(const uint8_t &fingers)
{
    libinputactions::InputBackend::instance()->setIgnoreEvents(true);
    PROTO::pointerGestures->pinchBegin(0, fingers);
    libinputactions::InputBackend::instance()->setIgnoreEvents(false);
}

void HyprlandInputEmitter::touchpadSwipeBegin(const uint8_t &fingers)
{
    libinputactions::InputBackend::instance()->setIgnoreEvents(true);
    g_pInputManager->onSwipeBegin(IPointer::SSwipeBeginEvent{
        .fingers = fingers
    });
    libinputactions::InputBackend::instance()->setIgnoreEvents(false);
}

const std::string &VirtualKeyboard::getName()
{
    static const std::string name = "inputactions_keyboard";
    return name;
}

bool VirtualPointer::isVirtual()
{
    return false;
}

SP<Aquamarine::IPointer> VirtualPointer::aq()
{
    return nullptr;
}