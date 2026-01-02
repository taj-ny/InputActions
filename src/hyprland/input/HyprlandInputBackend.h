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

#include "utils/HyprlandFunctionHook.h"
#include <hyprland/src/SharedDefs.hpp>
#include <hyprland/src/devices/IPointer.hpp>
#include <hyprland/src/plugins/HookSystem.hpp>
#undef HANDLE
#include <libinputactions/input/backends/LibinputInputBackend.h>

namespace InputActions
{

class InputDevice;

struct HyprlandInputDevice
{
    IHID *hyprlandDevice;
    std::unique_ptr<InputDevice> libinputactionsDevice;
    std::vector<CHyprSignalListener> listeners;
};

/**
 * Hold and pinch touchpad gestures require hooking, and therefore only work on x86_64.
 */
class HyprlandInputBackend : public LibinputInputBackend
{
public:
    HyprlandInputBackend(void *handle);
    ~HyprlandInputBackend() override;

    void initialize() override;
    void reset() override;

protected:
    void touchpadPinchBlockingStopped(uint32_t fingers) override;
    void touchpadSwipeBlockingStopped(uint32_t fingers) override;

private:
    void checkDeviceChanges();
    void deviceRemoved(const HyprlandInputDevice &device);

    void keyboardKey(SCallbackInfo &info, const std::any &data);

    static void holdBeginHook(void *thisPtr, uint32_t timeMs, uint32_t fingers);
    static void holdEndHook(void *thisPtr, uint32_t timeMs, bool cancelled);

    static void pinchBeginHook(void *thisPtr, uint32_t timeMs, uint32_t fingers);
    static void pinchUpdateHook(void *thisPtr, uint32_t timeMs, const Vector2D &delta, double scale, double rotation);
    static void pinchEndHook(void *thisPtr, uint32_t timeMs, bool cancelled);

    void touchpadSwipeBegin(SCallbackInfo &info, const std::any &data);
    void touchpadSwipeUpdate(SCallbackInfo &info, const std::any &data);
    void touchpadSwipeEnd(SCallbackInfo &info, const std::any &data);

    void pointerAxis(SCallbackInfo &info, const std::any &data);
    void pointerButton(SCallbackInfo &info, const std::any &data);
    void pointerMotion(SCallbackInfo &info, const std::any &data);

    HyprlandInputDevice *findDevice(IHID *hyprlandDevice);
    InputDevice *findInputActionsDevice(IHID *hyprlandDevice);

    std::vector<SP<HOOK_CALLBACK_FN>> m_events;

    /**
     * Used to detect device changes.
     */
    std::vector<WP<IHID>> m_previousHids;
    std::vector<HyprlandInputDevice> m_devices;
    QTimer m_deviceChangeTimer;

    /**
     * Mouse or touchpad.
     */
    InputDevice *m_currentPointingDevice{};
    InputDevice *m_currentTouchpad{};

    Vector2D m_previousPointerPosition;

    HyprlandFunctionHook m_holdBeginHook;
    HyprlandFunctionHook m_holdEndHook;
    HyprlandFunctionHook m_pinchBeginHook;
    HyprlandFunctionHook m_pinchUpdateHook;
    HyprlandFunctionHook m_pinchEndHook;
};

}