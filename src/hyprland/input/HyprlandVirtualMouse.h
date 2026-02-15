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

#include <hyprland/src/devices/IPointer.hpp>
#undef HANDLE
#include <libinputactions/input/devices/VirtualMouse.h>

namespace InputActions
{

class HyprlandVirtualMouse : public VirtualMouse
{
public:
    HyprlandVirtualMouse();
    ~HyprlandVirtualMouse() override;

    IPointer *hyprlandDevice() { return m_device.get(); }

    void mouseButton(MouseButton button, bool state) override;
    void mouseMotion(const QPointF &pos) override;
    void mouseWheel(const QPointF &delta) override;

private:
    class Device : public IPointer
    {
    public:
        Device();

        bool isVirtual() override { return false; }
        SP<Aquamarine::IPointer> aq() { return {}; }
    };
    SP<Device> m_device;
};

}