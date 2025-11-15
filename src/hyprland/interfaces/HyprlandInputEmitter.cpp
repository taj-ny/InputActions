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
#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/helpers/WLClasses.hpp>
#include <hyprland/src/managers/KeybindManager.hpp>
#include <hyprland/src/managers/SeatManager.hpp>
#include <hyprland/src/managers/input/InputManager.hpp>
#include <hyprland/src/protocols/core/Compositor.hpp>
#undef HANDLE
#include <libinputactions/input/InputDevice.h>
#include <libinputactions/input/backends/InputBackend.h>

using namespace InputActions;

HyprlandInputEmitter::HyprlandInputEmitter()
    : m_keyboard(makeShared<VirtualKeyboard>())
    , m_pointer(makeShared<VirtualPointer>())
{
    g_pInputManager->newKeyboard(m_keyboard);

    // FIXME: Text input list is private, inputs added before the plugin is loaded will not work
    m_listeners.push_back(PROTO::textInputV3->m_events.newTextInput.listen(std::bind(&HyprlandInputEmitter::onNewTextInputV3, this, std::placeholders::_1)));
}

HyprlandInputEmitter::~HyprlandInputEmitter()
{
    m_keyboard->events.destroy.emit();
}

void HyprlandInputEmitter::keyboardClearModifiers()
{
    g_inputBackend->setIgnoreEvents(true);
    m_modifiers = 0;
    const auto modifiers = g_inputBackend->keyboardModifiers();
    for (auto &keyboard : g_pInputManager->m_keyboards) {
        if (auto aqKeyboard = keyboard->aq()) {
            for (const auto &[key, modifier] : InputActions::KEYBOARD_MODIFIERS) {
                if (modifiers & modifier) {
                    aqKeyboard->events.key.emit(Aquamarine::IKeyboard::SKeyEvent{
                        .key = key,
                        .pressed = false,
                    });
                }
            }
            aqKeyboard->events.modifiers.emit(Aquamarine::IKeyboard::SModifiersEvent{
                .depressed = 0,
            });
        }
    }
    g_inputBackend->setIgnoreEvents(false);
}

void HyprlandInputEmitter::keyboardKey(uint32_t key, bool state, const InputDevice *device)
{
    g_inputBackend->setIgnoreEvents(true);
    if (const auto modifier = g_pKeybindManager->keycodeToModifier(key + 8)) {
        if (state) {
            m_modifiers |= modifier;
        } else {
            m_modifiers &= ~modifier;
        }
        m_keyboard->events.modifiers.emit(Aquamarine::IKeyboard::SModifiersEvent{
            .depressed = m_modifiers,
        });
    }

    m_keyboard->events.key.emit(Aquamarine::IKeyboard::SKeyEvent{
        .key = key,
        .pressed = state,
    });
    g_inputBackend->setIgnoreEvents(false);
}

void HyprlandInputEmitter::keyboardText(const QString &text)
{
    if (!g_pCompositor->m_lastFocus) {
        return;
    }

    const auto *client = g_pCompositor->m_lastFocus->client();
    for (const auto &[v3, _] : m_v3TextInputs) {
        if (v3->client() == client && v3->good()) {
            v3->preeditString({}, 0, 0);
            v3->commitString(text.toStdString());
            v3->sendDone();
            return;
        }
    }
}

void HyprlandInputEmitter::mouseButton(uint32_t button, bool state, const InputDevice *device)
{
    g_inputBackend->setIgnoreEvents(true);
    g_pInputManager->onMouseButton(IPointer::SButtonEvent{
        .button = button,
        .state = state ? WL_POINTER_BUTTON_STATE_PRESSED : WL_POINTER_BUTTON_STATE_RELEASED,
    });
    g_inputBackend->setIgnoreEvents(false);
}

void HyprlandInputEmitter::mouseMoveRelative(const QPointF &pos)
{
    g_inputBackend->setIgnoreEvents(true);
    const Vector2D delta(pos.x(), pos.y());
    g_pInputManager->onMouseMoved(IPointer::SMotionEvent{
        .delta = delta,
        .unaccel = delta,
        .device = m_pointer,
    });
    g_inputBackend->setIgnoreEvents(false);
}

void HyprlandInputEmitter::onNewTextInputV3(const WP<CTextInputV3> &textInput)
{
    m_v3TextInputs.emplace_back(textInput, textInput->m_events.destroy.listen([this, textInput]() {
        std::erase_if(m_v3TextInputs, [textInput](const auto &input) {
            return input.first == textInput;
        });
    }));
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