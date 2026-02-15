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

#include <hyprland/src/SharedDefs.hpp>
#include <hyprland/src/devices/IHID.hpp>
#include <hyprland/src/devices/IPointer.hpp>
#include <hyprland/src/devices/ITouch.hpp>
#include <hyprland/src/plugins/HookSystem.hpp>
#undef HANDLE
#include <libinputactions/input/devices/InputDevice.h>

namespace InputActions
{

class HyprlandInputBackend;

class HyprlandInputDevice : public InputDevice
{
public:
    static std::unique_ptr<HyprlandInputDevice> tryCreate(HyprlandInputBackend *backend, SP<IHID> device);

    IHID *hyprlandDevice() const { return m_device.get(); }

    void keyboardKey(KeyboardKey key, bool state) override;
    void mouseButton(MouseButton button, bool state) override;

    void resetVirtualDeviceState() override;
    void restoreVirtualDeviceState() override;

protected:
    void touchscreenTapDown(const std::vector<QPointF> &points) override;
    void touchscreenTapUp(const std::vector<QPointF> &points) override;

private:
    HyprlandInputDevice(SP<IHID> device, InputDeviceType type, const std::string &name, HyprlandInputBackend *backend);
    HyprlandInputDevice(SP<ITouch> device, InputDeviceType type, const std::string &name, HyprlandInputBackend *backend);
    HyprlandInputDevice(SP<IPointer> device, InputDeviceType type, const std::string &name, HyprlandInputBackend *backend);

    SP<IHID> m_device;
    std::vector<CHyprSignalListener> m_listeners;
    HyprlandInputBackend *m_backend;
};

}