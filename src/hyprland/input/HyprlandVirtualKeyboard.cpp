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

#include "HyprlandVirtualKeyboard.h"
#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/desktop/state/FocusState.hpp>
#include <hyprland/src/managers/KeybindManager.hpp>
#include <hyprland/src/managers/input/InputManager.hpp>
#include <hyprland/src/protocols/core/Compositor.hpp>
#undef HANDLE
#include <libinputactions/input/backends/InputBackend.h>

namespace InputActions
{

HyprlandVirtualKeyboard::HyprlandVirtualKeyboard()
    : m_device(makeShared<Device>())
{
    g_pInputManager->newKeyboard(m_device);

    // FIXME: Text input list is private, inputs added before the plugin is loaded will not work
    m_newTextInputV3Listener = PROTO::textInputV3->m_events.newTextInput.listen([this](const auto &textInput) {
        onNewTextInputV3(textInput);
    });
}

HyprlandVirtualKeyboard::~HyprlandVirtualKeyboard()
{
    reset();
    m_device->events.destroy.emit();
}

void HyprlandVirtualKeyboard::keyboardKey(uint32_t key, bool state)
{
    g_inputBackend->setIgnoreEvents(true);

    m_device->events.key.emit(Aquamarine::IKeyboard::SKeyEvent{
        .key = key,
        .pressed = state,
    });
    VirtualKeyboard::keyboardKey(key, state);

    if (const auto modifier = g_pKeybindManager->keycodeToModifier(key + 8)) {
        if (state) {
            m_modifiers |= modifier;
        } else {
            m_modifiers &= ~modifier;
        }
        m_device->events.modifiers.emit(Aquamarine::IKeyboard::SModifiersEvent{
            .depressed = m_modifiers,
        });
    }

    g_inputBackend->setIgnoreEvents(false);
}

void HyprlandVirtualKeyboard::keyboardText(const QString &text)
{
    if (!Desktop::focusState()->window()) {
        return;
    }

    const auto *client = Desktop::focusState()->surface()->client();
    for (const auto &[v3, _] : m_v3TextInputs) {
        if (v3->client() == client && v3->good()) {
            v3->preeditString({}, 0, 0);
            v3->commitString(text.toStdString());
            v3->sendDone();
            return;
        }
    }
}

void HyprlandVirtualKeyboard::onNewTextInputV3(const WP<CTextInputV3> &textInput)
{
    m_v3TextInputs.emplace_back(textInput, textInput->m_events.destroy.listen([this, textInput]() {
        std::erase_if(m_v3TextInputs, [textInput](const auto &input) {
            return input.first == textInput;
        });
    }));
}

const std::string &HyprlandVirtualKeyboard::Device::getName()
{
    static const std::string name = "inputactions-virtual-keyboard";
    return name;
}

}