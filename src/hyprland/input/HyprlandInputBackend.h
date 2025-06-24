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

#include <libinputactions/input/backends/LibinputCompositorInputBackend.h>

#include <hyprland/src/devices/IPointer.hpp>
#include <hyprland/src/plugins/HookSystem.hpp>
#include <hyprland/src/SharedDefs.hpp>
#undef HANDLE

class Plugin;

struct HyprlandInputDevice
{
    IHID *hyprlandDevice;
    std::unique_ptr<libinputactions::InputDevice> libinputactionsDevice;
    std::vector<CHyprSignalListener> listeners;
};

/**
 * Hold and pinch touchpad gestures require hooking, and therefore only work on x86_64.
 */
class HyprlandInputBackend : public QObject, public libinputactions::LibinputCompositorInputBackend
{
public:
    HyprlandInputBackend(Plugin *plugin);
    ~HyprlandInputBackend() override;

    void initialize() override;
    void reset() override;

private:
    void checkDeviceChanges();

    void keyboardKey(SCallbackInfo &info, const std::any &data);

    bool touchpadHoldBegin(const uint8_t &fingers);
    bool touchpadHoldEnd(const bool &cancelled);

    bool touchpadPinchBegin(const uint8_t &fingers);
    bool touchpadPinchUpdate(const double &scale, const double &angleDelta);
    bool touchpadPinchEnd(const bool &cancelled);

    void touchpadSwipeBegin(SCallbackInfo &info, const std::any &data);
    void touchpadSwipeUpdate(SCallbackInfo &info, const std::any &data);
    void touchpadSwipeEnd(SCallbackInfo &info, const std::any &data);

    void pointerAxis(SCallbackInfo &info, const std::any &data);
    void pointerButton(SCallbackInfo &info, const std::any &data);
    void pointerMotion(SCallbackInfo &info, const std::any &data);

    HyprlandInputDevice *findDevice(IHID *hyprlandDevice);
    libinputactions::InputDevice *findInputActionsDevice(IHID *hyprlandDevice);

    std::vector<SP<HOOK_CALLBACK_FN>> m_events;

    /**
     * Used to detect device changes.
     */
    std::vector<IHID *> m_hyprlandDevices;
    std::vector<HyprlandInputDevice> m_devices;
    QTimer m_deviceChangeTimer;

    /**
     * Mouse or touchpad.
     */
    libinputactions::InputDevice *m_currentPointingDevice{};
    libinputactions::InputDevice *m_currentTouchpad{};

    Vector2D m_previousPointerPosition;

    // Unhooked by Hyprland on plugin unload
    CFunctionHook *m_holdBeginHook;
    CFunctionHook *m_holdEndHook;
    CFunctionHook *m_pinchBeginHook;
    CFunctionHook *m_pinchUpdateHook;
    CFunctionHook *m_pinchEndHook;

    friend void holdBeginHook(void *thisPtr, uint32_t timeMs, uint32_t fingers);
    friend void holdEndHook(void *thisPtr, uint32_t timeMs, bool cancelled);
    friend void pinchBeginHook(void *thisPtr, uint32_t timeMs, uint32_t fingers);
    friend void pinchUpdateHook(void *thisPtr, uint32_t timeMs, const Vector2D &delta, double scale, double rotation);
    friend void pinchEndHook(void *thisPtr, uint32_t timeMs, bool cancelled);
};