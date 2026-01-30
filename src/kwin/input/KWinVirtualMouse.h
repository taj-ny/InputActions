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
#include <libinputactions/input/devices/VirtualMouse.h>

namespace InputActions
{

class KWinVirtualMouse : public VirtualMouse
{
public:
    KWinVirtualMouse();
    ~KWinVirtualMouse() override;

    KWin::InputDevice *kwinDevice() { return &m_device; }

    void mouseButton(uint32_t button, bool state) override;
    void mouseMotion(const QPointF &pos) override;
    void mouseWheel(const QPointF &delta) override;

private:
    class Device : public KWin::InputDevice
    {
    public:
        QString name() const override { return "InputActions Virtual Mouse"; }
        bool isEnabled() const override { return true; }
        void setEnabled(bool enabled) override {}
        bool isKeyboard() const override { return false; }
        bool isPointer() const override { return true; }
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