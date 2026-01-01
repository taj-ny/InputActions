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

#include "HyprlandInputBackend.h"
#include "Plugin.h"
#include "interfaces/HyprlandInputEmitter.h"
#include <aquamarine/input/Input.hpp>
#include <hyprland/src/config/ConfigManager.hpp>
#include <hyprland/src/devices/IKeyboard.hpp>
#include <hyprland/src/devices/IPointer.hpp>
#include <hyprland/src/managers/SeatManager.hpp>
#include <hyprland/src/managers/input/InputManager.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/protocols/PointerGestures.hpp>
#undef HANDLE
#include <libinputactions/input/InputDevice.h>

using namespace InputActions;

typedef void (*holdBegin)(void *thisPtr, uint32_t timeMs, uint32_t fingers);
typedef void (*holdEnd)(void *thisPtr, uint32_t timeMs, bool cancelled);
typedef void (*pinchBegin)(void *thisPtr, uint32_t timeMs, uint32_t fingers);
typedef void (*pinchUpdate)(void *thisPtr, uint32_t timeMs, const Vector2D &delta, double scale, double rotation);
typedef void (*pinchEnd)(void *thisPtr, uint32_t timeMs, bool cancelled);

HyprlandInputBackend::HyprlandInputBackend(void *handle)
    : m_holdBeginHook(handle, "holdBegin", (void *)&holdBeginHook)
    , m_holdEndHook(handle, "holdEnd", (void *)&holdEndHook)
    , m_pinchBeginHook(handle, "pinchBegin", (void *)pinchBeginHook)
    , m_pinchUpdateHook(handle, "pinchUpdate", (void *)pinchUpdateHook)
    , m_pinchEndHook(handle, "pinchEnd", (void *)&pinchEndHook)
{
    m_events.push_back(HyprlandAPI::registerCallbackDynamic(handle, "keyPress", [this](void *, SCallbackInfo &info, std::any data) {
        keyboardKey(info, data);
    }));
    m_events.push_back(HyprlandAPI::registerCallbackDynamic(handle, "mouseAxis", [this](void *, SCallbackInfo &info, std::any data) {
        pointerAxis(info, data);
    }));
    m_events.push_back(HyprlandAPI::registerCallbackDynamic(handle, "mouseButton", [this](void *, SCallbackInfo &info, std::any data) {
        pointerButton(info, data);
    }));
    m_events.push_back(HyprlandAPI::registerCallbackDynamic(handle, "mouseMove", [this](void *, SCallbackInfo &info, std::any data) {
        pointerMotion(info, data);
    }));
    m_events.push_back(HyprlandAPI::registerCallbackDynamic(handle, "swipeBegin", [this](void *, SCallbackInfo &info, std::any data) {
        touchpadSwipeBegin(info, data);
    }));
    m_events.push_back(HyprlandAPI::registerCallbackDynamic(handle, "swipeUpdate", [this](void *, SCallbackInfo &info, std::any data) {
        touchpadSwipeUpdate(info, data);
    }));
    m_events.push_back(HyprlandAPI::registerCallbackDynamic(handle, "swipeEnd", [this](void *, SCallbackInfo &info, std::any data) {
        touchpadSwipeEnd(info, data);
    }));

    connect(&m_deviceChangeTimer, &QTimer::timeout, this, &HyprlandInputBackend::checkDeviceChanges);
    m_deviceChangeTimer.setInterval(1000);
    m_deviceChangeTimer.start();
}

HyprlandInputBackend::~HyprlandInputBackend()
{
    reset();
}

void HyprlandInputBackend::initialize()
{
    LibinputInputBackend::initialize();

    checkDeviceChanges();
}

void HyprlandInputBackend::reset()
{
    for (const auto &device : m_devices) {
        deviceRemoved(device);
    }
    m_devices.clear();
    LibinputInputBackend::reset();
}

void HyprlandInputBackend::touchpadPinchBlockingStopped(uint32_t fingers)
{
    m_ignoreEvents = true;
    PROTO::pointerGestures->pinchBegin(0, fingers);
    m_ignoreEvents = false;
}

void HyprlandInputBackend::touchpadSwipeBlockingStopped(uint32_t fingers)
{
    m_ignoreEvents = true;
    g_pInputManager->onSwipeBegin(IPointer::SSwipeBeginEvent{
        .fingers = fingers,
    });
    m_ignoreEvents = false;
}

void HyprlandInputBackend::checkDeviceChanges()
{
    auto &hids = g_pInputManager->m_hids;
    for (auto &device : hids) {
        if (std::ranges::any_of(m_previousHids, [&device](const auto &existingDevice) {
                return existingDevice.get() == device.get();
            })) {
            continue;
        }

        HyprlandInputDevice newDevice{
            .hyprlandDevice = device.get(),
        };
        InputDeviceType type;
        QString name;
        if (auto *keyboard = dynamic_cast<IKeyboard *>(device.get())) {
            if (keyboard->aq().get() == std::dynamic_pointer_cast<HyprlandInputEmitter>(g_inputEmitter)->keyboard()) {
                continue;
            }

            type = InputDeviceType::Keyboard;
            name = QString::fromStdString(keyboard->m_deviceName);
        } else if (auto *pointer = dynamic_cast<IPointer *>(device.get())) {
            type = pointer->m_isTouchpad ? InputDeviceType::Touchpad : InputDeviceType::Mouse;
            name = QString::fromStdString(pointer->m_deviceName);

            // Not all events provide the device, so it is instead obtained by listening to events of all devices. Those listeners are executed after the event
            // is handled by the backend, so the first libinputactions event after launching the compositor will have a null sender and after changing the
            // input device it will have the previous one of the same type.
            auto &events = pointer->m_pointerEvents;
            if (pointer->m_isTouchpad) {
                const auto listener = [this, device](const auto &) {
                    if (auto *foundDevice = findDevice(device.get())) {
                        m_currentPointingDevice = foundDevice->libinputactionsDevice.get();
                        m_currentTouchpad = m_currentPointingDevice;
                    }
                };
                newDevice.listeners.push_back(events.axis.registerListener(listener));
                newDevice.listeners.push_back(events.button.registerListener(listener));
                newDevice.listeners.push_back(events.motion.registerListener(listener));
                newDevice.listeners.push_back(events.holdBegin.registerListener(listener));
                newDevice.listeners.push_back(events.pinchBegin.registerListener(listener));
                newDevice.listeners.push_back(events.swipeBegin.registerListener(listener));
            } else {
                const auto listener = [this, device](const auto &) {
                    if (auto *foundDevice = findDevice(device.get())) {
                        m_currentPointingDevice = foundDevice->libinputactionsDevice.get();
                    }
                };
                newDevice.listeners.push_back(events.axis.registerListener(listener));
                newDevice.listeners.push_back(events.button.registerListener(listener));
                newDevice.listeners.push_back(events.motion.registerListener(listener));
            }
        } else {
            continue;
        }

        newDevice.libinputactionsDevice = std::make_unique<InputDevice>(type, name);
        if (g_pConfigManager->getDeviceString(device->m_deviceName, "tap_button_map", "input:touchpad:tap_button_map") == "lmr") {
            newDevice.libinputactionsDevice->properties().setLmrTapButtonMap(true);
        }

        deviceAdded(newDevice.libinputactionsDevice.get());
        m_devices.push_back(std::move(newDevice));
    }
    m_previousHids = hids;

    for (auto it = m_devices.begin(); it != m_devices.end();) {
        if (!std::ranges::any_of(hids, [it](const auto &device) {
                return device.get() == it->hyprlandDevice;
            })) {
            deviceRemoved(*it);
            it = m_devices.erase(it);
            continue;
        }
        it++;
    }
}

void HyprlandInputBackend::deviceRemoved(const HyprlandInputDevice &device)
{
    if (m_currentPointingDevice == device.libinputactionsDevice.get()) {
        m_currentPointingDevice = nullptr;
    }
    if (m_currentTouchpad == device.libinputactionsDevice.get()) {
        m_currentTouchpad = nullptr;
    }
    LibinputInputBackend::deviceRemoved(device.libinputactionsDevice.get());
}

void HyprlandInputBackend::keyboardKey(SCallbackInfo &info, const std::any &data)
{
    if (m_ignoreEvents) {
        return;
    }

    const auto map = std::any_cast<std::unordered_map<std::string, std::any>>(data);
    const auto event = std::any_cast<IKeyboard::SKeyEvent>(map.at("event"));
    const auto keyboard = std::any_cast<SP<IKeyboard>>(map.at("keyboard"));
    const auto state = event.state == WL_KEYBOARD_KEY_STATE_PRESSED;

    auto *device = findInputActionsDevice(keyboard.get());
    if (device) {
        device->setKeyState(event.keycode, state);
    }
    info.cancelled = LibinputInputBackend::keyboardKey(device, event.keycode, state);
}

void HyprlandInputBackend::pointerAxis(SCallbackInfo &info, const std::any &data)
{
    const auto map = std::any_cast<std::unordered_map<std::string, std::any>>(data);
    const auto event = std::any_cast<IPointer::SAxisEvent>(map.at("event"));

    auto delta = event.axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL ? QPointF(event.delta, 0) : QPointF(0, event.delta);
    if (event.relativeDirection == WL_POINTER_AXIS_RELATIVE_DIRECTION_INVERTED) {
        delta *= -1;
    }
    info.cancelled = LibinputInputBackend::pointerAxis(m_currentPointingDevice, delta, true);
}

void HyprlandInputBackend::pointerButton(SCallbackInfo &info, const std::any &data)
{
    const auto event = std::any_cast<IPointer::SButtonEvent>(data);
    info.cancelled = LibinputInputBackend::pointerButton(m_currentPointingDevice,
                                                         scanCodeToMouseButton(event.button),
                                                         event.button,
                                                         event.state == WL_POINTER_BUTTON_STATE_PRESSED);
}

void HyprlandInputBackend::pointerMotion(SCallbackInfo &info, const std::any &data)
{
    const auto pointerPosition = std::any_cast<const Vector2D>(data);
    const auto delta = pointerPosition - m_previousPointerPosition;
    m_previousPointerPosition = pointerPosition;
    info.cancelled = LibinputInputBackend::pointerMotion(m_currentPointingDevice, {{delta.x, delta.y}});
}

void HyprlandInputBackend::holdBeginHook(void *thisPtr, uint32_t timeMs, uint32_t fingers)
{
    auto *self = dynamic_cast<HyprlandInputBackend *>(g_inputBackend.get());
    if (!self->LibinputInputBackend::touchpadHoldBegin(self->m_currentTouchpad, fingers)) {
        (*(holdBegin)self->m_holdBeginHook->m_original)(thisPtr, timeMs, fingers);
    }
}

void HyprlandInputBackend::holdEndHook(void *thisPtr, uint32_t timeMs, bool cancelled)
{
    auto *self = dynamic_cast<HyprlandInputBackend *>(g_inputBackend.get());
    if (!self->LibinputInputBackend::touchpadHoldEnd(self->m_currentTouchpad, cancelled)) {
        (*(holdEnd)self->m_holdEndHook->m_original)(thisPtr, timeMs, cancelled);
    }
}

void HyprlandInputBackend::pinchBeginHook(void *thisPtr, uint32_t timeMs, uint32_t fingers)
{
    auto *self = dynamic_cast<HyprlandInputBackend *>(g_inputBackend.get());
    if (!self->LibinputInputBackend::touchpadPinchBegin(self->m_currentTouchpad, fingers)) {
        (*(pinchBegin)self->m_pinchBeginHook->m_original)(thisPtr, timeMs, fingers);
    }
}

void HyprlandInputBackend::pinchUpdateHook(void *thisPtr, uint32_t timeMs, const Vector2D &delta, double scale, double rotation)
{
    auto *self = dynamic_cast<HyprlandInputBackend *>(g_inputBackend.get());
    if (!self->LibinputInputBackend::touchpadPinchUpdate(self->m_currentTouchpad, scale, rotation)) {
        (*(pinchUpdate)self->m_pinchUpdateHook->m_original)(thisPtr, timeMs, delta, scale, rotation);
    }
}

void HyprlandInputBackend::pinchEndHook(void *thisPtr, uint32_t timeMs, bool cancelled)
{
    auto *self = dynamic_cast<HyprlandInputBackend *>(g_inputBackend.get());
    if (!self->LibinputInputBackend::touchpadPinchEnd(self->m_currentTouchpad, cancelled)) {
        (*(pinchEnd)self->m_pinchEndHook->m_original)(thisPtr, timeMs, cancelled);
    }
}

void HyprlandInputBackend::touchpadSwipeBegin(SCallbackInfo &info, const std::any &data)
{
    const auto event = std::any_cast<IPointer::SSwipeBeginEvent>(data);
    info.cancelled = LibinputInputBackend::touchpadSwipeBegin(m_currentTouchpad, event.fingers);
}

void HyprlandInputBackend::touchpadSwipeUpdate(SCallbackInfo &info, const std::any &data)
{
    const auto event = std::any_cast<IPointer::SSwipeUpdateEvent>(data);
    info.cancelled = LibinputInputBackend::touchpadSwipeUpdate(m_currentTouchpad, {{event.delta.x, event.delta.y}});
}

void HyprlandInputBackend::touchpadSwipeEnd(SCallbackInfo &info, const std::any &data)
{
    const auto event = std::any_cast<IPointer::SSwipeEndEvent>(data);
    info.cancelled = LibinputInputBackend::touchpadSwipeEnd(m_currentTouchpad, event.cancelled);
}

HyprlandInputDevice *HyprlandInputBackend::findDevice(IHID *hyprlandDevice)
{
    for (auto &device : m_devices) {
        if (device.hyprlandDevice == hyprlandDevice) {
            return &device;
        }
    }
    return {};
}

InputDevice *HyprlandInputBackend::findInputActionsDevice(IHID *hyprlandDevice)
{
    if (auto *device = findDevice(hyprlandDevice)) {
        return device->libinputactionsDevice.get();
    }
    return {};
}