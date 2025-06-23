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

#include <libinputactions/interfaces/InputEmitter.h>

#include <aquamarine/input/Input.hpp>
#include <hyprland/src/devices/IKeyboard.hpp>
#include <hyprland/src/devices/IPointer.hpp>
#undef HANDLE

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

    bool isVirtual();
    SP<Aquamarine::IPointer> aq();
};

class HyprlandInputEmitter : public libinputactions::InputEmitter
{
public:
    HyprlandInputEmitter();
    ~HyprlandInputEmitter() override;

    void keyboardClearModifiers() override;
    void keyboardKey(const uint32_t &key, const bool &state) override;

    void mouseButton(const uint32_t &button, const bool &state) override;
    void mouseMoveRelative(const QPointF &pos) override;

    void touchpadPinchBegin(const uint8_t &fingers) override;
    void touchpadSwipeBegin(const uint8_t &fingers) override;

private:
    uint32_t m_modifiers{};
    SP<Aquamarine::IKeyboard> m_keyboard;
    SP<IPointer> m_pointer;
};