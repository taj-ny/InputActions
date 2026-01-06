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

#include <aquamarine/input/Input.hpp>
#include <hyprland/src/devices/IKeyboard.hpp>
#include <hyprland/src/devices/IPointer.hpp>
#include <hyprland/src/protocols/TextInputV3.hpp>
#include <libinputactions/interfaces/InputEmitter.h>

namespace InputActions
{

class VirtualKeyboard : public Aquamarine::IKeyboard
{
public:
    VirtualKeyboard() = default;

    const std::string &getName() override;
};

class VirtualPointer : public IPointer
{
public:
    VirtualPointer() = default;

    bool isVirtual() override;
    SP<Aquamarine::IPointer> aq() override;
};

class HyprlandInputEmitter : public InputEmitter
{
public:
    HyprlandInputEmitter();
    ~HyprlandInputEmitter() override;

    void keyboardClearModifiers() override;
    void keyboardKey(uint32_t key, bool state, InputDevice *device = nullptr) override;
    void keyboardText(const QString &text) override;

    void mouseButton(uint32_t button, bool state, InputDevice *device = nullptr) override;
    void mouseMoveRelative(const QPointF &pos) override;

    Aquamarine::IKeyboard *keyboard() const { return m_keyboard.get(); }

private:
    void onNewTextInputV3(const WP<CTextInputV3> &textInput);

    uint32_t m_modifiers{};
    SP<Aquamarine::IKeyboard> m_keyboard;
    SP<IPointer> m_pointer;

    std::vector<std::pair<WP<CTextInputV3>, CHyprSignalListener>> m_v3TextInputs;
    std::vector<CHyprSignalListener> m_listeners;
};

}