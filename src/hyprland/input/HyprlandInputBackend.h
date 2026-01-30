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

#include "HyprlandInputDevice.h"
#include "HyprlandVirtualKeyboard.h"
#include "HyprlandVirtualMouse.h"
#include "utils/HyprlandFunctionHook.h"
#include <hyprland/src/SharedDefs.hpp>
#include <hyprland/src/devices/IPointer.hpp>
#include <hyprland/src/plugins/HookSystem.hpp>
#undef HANDLE
#include <libinputactions/input/backends/LibinputInputBackend.h>

namespace InputActions
{

class InputDevice;

/**
 * Uses three different methods for getting events, because Hyprland does not always provide senders.
 *   1. Hyprland events (HyprlandAPI::registerCallbackDynamic) for keyboard key press and touch down events. The sender is provided in the event itself.
 *   2. Function hooks for pointer axis and pointer button events. The sender is provided as an argument alongside the event.
 *   3. Function hooks + signals for all other events. The sender is not provided at all. To get the device, the backend blocks the call by default, then,
 *      once it gets the event and sender from the signal, it re-emits the signal.
 */
class HyprlandInputBackend : public LibinputInputBackend
{
public:
    HyprlandInputBackend(void *handle);
    ~HyprlandInputBackend() override;

    VirtualKeyboard *virtualKeyboard() override;
    VirtualMouse *virtualMouse() override;

    void initialize() override;
    void reset() override;

    void onHoldBeginSignal(InputDevice *sender, IPointer *hyprlandDevice, const IPointer::SHoldBeginEvent &event);
    void onHoldEndSignal(InputDevice *sender, IPointer *hyprlandDevice, const IPointer::SHoldEndEvent &event);
    void onPinchBeginSignal(InputDevice *sender, IPointer *hyprlandDevice, const IPointer::SPinchBeginEvent &event);
    void onPinchUpdateSignal(InputDevice *sender, IPointer *hyprlandDevice, const IPointer::SPinchUpdateEvent &event);
    void onPinchEndSignal(InputDevice *sender, IPointer *hyprlandDevice, const IPointer::SPinchEndEvent &event);
    void onPointerButtonSignal(InputDevice *sender, IPointer *hyprlandDevice, const IPointer::SButtonEvent &event);
    void onSwipeBeginSignal(InputDevice *sender, IPointer *hyprlandDevice, const IPointer::SSwipeBeginEvent &event);
    void onSwipeUpdateSignal(InputDevice *sender, IPointer *hyprlandDevice, const IPointer::SSwipeUpdateEvent &event);
    void onSwipeEndSignal(InputDevice *sender, IPointer *hyprlandDevice, const IPointer::SSwipeEndEvent &event);
    void onTouchCancelSignal(InputDevice *sender, ITouch *hyprlandDevice, const ITouch::SCancelEvent &event);
    void onTouchFrameSignal(InputDevice *sender, ITouch *hyprlandDevice);
    void onTouchMotionSignal(InputDevice *sender, ITouch *hyprlandDevice, const ITouch::SMotionEvent &event);
    void onTouchUpSignal(InputDevice *sender, ITouch *hyprlandDevice, const ITouch::SUpEvent &event);

protected:
    void touchpadPinchBlockingStopped(uint32_t fingers) override;
    void touchpadSwipeBlockingStopped(uint32_t fingers) override;

private:
    void checkDeviceChanges();
    void deviceRemoved(const HyprlandInputDevice *device);

    // Method 1
    void keyboardKey(SCallbackInfo &info, const std::any &data);
    void touchDown(SCallbackInfo &info, const std::any &data);

    // Method 2
    static void pointerAxisHook(void *thisPtr, IPointer::SAxisEvent event, SP<IPointer> sender);
    static void pointerMotionHook(void *thisPtr, IPointer::SMotionEvent event);

    // Method 3
    static void holdBeginHook(void *thisPtr, uint32_t timeMs, uint32_t fingers);
    static void holdEndHook(void *thisPtr, uint32_t timeMs, bool cancelled);
    static void pinchBeginHook(void *thisPtr, uint32_t timeMs, uint32_t fingers);
    static void pinchUpdateHook(void *thisPtr, uint32_t timeMs, const Vector2D &delta, double scale, double rotation);
    static void pinchEndHook(void *thisPtr, uint32_t timeMs, bool cancelled);
    static void pointerButtonHook(void *thisPtr, IPointer::SButtonEvent event);
    static void swipeBeginHook(void *thisPtr, uint32_t timeMs, uint32_t fingers);
    static void swipeUpdateHook(void *thisPtr, uint32_t timeMs, const Vector2D &delta);
    static void swipeEndHook(void *thisPtr, uint32_t timeMs, bool cancelled);
    static void touchMotionHook(void *thisPtr, ITouch::SMotionEvent event);
    static void touchUpHook(void *thisPtr, ITouch::SUpEvent event);

    InputDevice *findInputActionsDevice(IHID *hyprlandDevice);

    /**
     * Used to detect device changes.
     */
    std::vector<WP<IHID>> m_previousHids;
    std::vector<std::unique_ptr<HyprlandInputDevice>> m_devices;
    QTimer m_deviceChangeTimer;

    std::optional<HyprlandVirtualKeyboard> m_virtualKeyboard;
    std::optional<HyprlandVirtualMouse> m_virtualMouse;

    std::vector<SP<HOOK_CALLBACK_FN>> m_eventListeners;

    bool m_blockHookCalls{};
    HyprlandFunctionHook<&holdBeginHook> m_holdBeginHook;
    HyprlandFunctionHook<&holdEndHook> m_holdEndHook;
    HyprlandFunctionHook<&pinchBeginHook> m_pinchBeginHook;
    HyprlandFunctionHook<&pinchUpdateHook> m_pinchUpdateHook;
    HyprlandFunctionHook<&pinchEndHook> m_pinchEndHook;
    HyprlandFunctionHook<&pointerAxisHook> m_pointerAxisHook;
    HyprlandFunctionHook<&pointerButtonHook> m_pointerButtonHook;
    HyprlandFunctionHook<&pointerMotionHook> m_pointerMotionHook;
    HyprlandFunctionHook<&swipeBeginHook> m_swipeBeginHook;
    HyprlandFunctionHook<&swipeUpdateHook> m_swipeUpdateHook;
    HyprlandFunctionHook<&swipeEndHook> m_swipeEndHook;
    HyprlandFunctionHook<&touchMotionHook> m_touchMotionHook;
    HyprlandFunctionHook<&touchUpHook> m_touchUpHook;
};

}