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

#include "input.h"
#include <libinputactions/input/devices/VirtualKeyboard.h>

namespace InputActions
{

class KWinVirtualKeyboard : public VirtualKeyboard
{
public:
    KWinVirtualKeyboard();
    ~KWinVirtualKeyboard() override;

    KWin::InputDevice *kwinDevice() { return &m_device; }

    void keyboardKey(KeyboardKey key, bool state) override;
    void keyboardText(const QString &text) override;

private:
    class Device : public KWin::InputDevice
    {
    public:
        QString name() const override { return "InputActions Virtual Keyboard"; }
        bool isEnabled() const override { return true; }
        void setEnabled(bool enabled) override {}
        bool isKeyboard() const override { return true; }
        bool isPointer() const override { return false; }
        bool isTouchpad() const override { return false; }
        bool isTouch() const override { return false; }
        bool isTabletTool() const override { return false; }
        bool isTabletPad() const override { return false; }
        bool isTabletModeSwitch() const override { return false; }
        bool isLidSwitch() const override { return false; }
    };
    Device m_device;
};

}