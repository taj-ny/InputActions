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

#include <aquamarine/input/Input.hpp>
#include <hyprland/src/protocols/TextInputV3.hpp>
#undef HANDLE
#include <libinputactions/input/devices/VirtualKeyboard.h>

namespace InputActions
{

class HyprlandVirtualKeyboard : public VirtualKeyboard
{
public:
    HyprlandVirtualKeyboard();
    ~HyprlandVirtualKeyboard() override;

    Aquamarine::IKeyboard *hyprlandDevice() { return m_device.get(); }

    void keyboardKey(KeyboardKey key, bool state) override;
    void keyboardText(const QString &text) override;

private:
    void onNewTextInputV3(const WP<CTextInputV3> &textInput);

    class Device : public Aquamarine::IKeyboard
    {
    public:
        Device() = default;

        const std::string &getName() override;
    };
    SP<Device> m_device;
    uint32_t m_modifiers{};

    std::vector<std::pair<WP<CTextInputV3>, CHyprSignalListener>> m_v3TextInputs;
    CHyprSignalListener m_newTextInputV3Listener;
};

}