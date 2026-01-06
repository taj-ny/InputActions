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
#include <libinputactions/input/InputDevice.h>

namespace InputActions
{

class KWinInputBackend;

class KWinInputDevice : public InputDevice
{
public:
    static std::unique_ptr<KWinInputDevice> tryCreate(KWinInputBackend *backend, KWin::InputDevice *device);

    KWin::InputDevice *kwinDevice() const { return m_kwinDevice; }

#ifdef KWIN_6_5_OR_GREATER
    void resetVirtualDeviceState() override;
    void restoreVirtualDeviceState() override;

protected:
    void simulateTouchscreenTapDown(const std::vector<QPointF> &points) override;
    void simulateTouchscreenTapUp(const std::vector<QPointF> &points) override;
#endif

private:
    KWinInputDevice(KWinInputBackend *backend, KWin::InputDevice *device, InputDeviceType type);

    KWin::InputDevice *m_kwinDevice;
    KWinInputBackend *m_backend;
};

}