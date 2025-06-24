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

#include <aquamarine/input/Input.hpp>
#include <hyprland/src/devices/IKeyboard.hpp>
#include <hyprland/src/devices/IPointer.hpp>
#include <hyprland/src/managers/input/InputManager.hpp>
#include <hyprland/src/managers/SeatManager.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>
#undef HANDLE

using namespace libinputactions;

typedef void (*holdBegin)(void *thisPtr, uint32_t timeMs, uint32_t fingers);
typedef void (*holdEnd)(void *thisPtr, uint32_t timeMs, bool cancelled);
typedef void (*pinchBegin)(void *thisPtr, uint32_t timeMs, uint32_t fingers);
typedef void (*pinchUpdate)(void *thisPtr, uint32_t timeMs, const Vector2D &delta, double scale, double rotation);
typedef void (*pinchEnd)(void *thisPtr, uint32_t timeMs, bool cancelled);

void holdBeginHook(void *thisPtr, uint32_t timeMs, uint32_t fingers)
{
    auto *instance = dynamic_cast<HyprlandInputBackend *>(InputBackend::instance());
    if (!instance->touchpadHoldBegin(fingers)) {
        (*(holdBegin)instance->m_holdBeginHook->m_original)(thisPtr, timeMs, fingers);
    }
}

void holdEndHook(void *thisPtr, uint32_t timeMs, bool cancelled)
{
    auto *instance = dynamic_cast<HyprlandInputBackend *>(InputBackend::instance());
    if (!instance->touchpadHoldEnd(cancelled)) {
        (*(holdEnd)instance->m_holdEndHook->m_original)(thisPtr, timeMs, cancelled);
    }
}

void pinchBeginHook(void *thisPtr, uint32_t timeMs, uint32_t fingers)
{
    auto *instance = dynamic_cast<HyprlandInputBackend *>(InputBackend::instance());
    if (!instance->touchpadPinchBegin(fingers)) {
        (*(pinchBegin)instance->m_pinchBeginHook->m_original)(thisPtr, timeMs, fingers);
    }
}

void pinchUpdateHook(void *thisPtr, uint32_t timeMs, const Vector2D &delta, double scale, double rotation)
{
    auto *instance = dynamic_cast<HyprlandInputBackend *>(InputBackend::instance());
    if (!instance->touchpadPinchUpdate(scale, rotation)) {
        (*(pinchUpdate)instance->m_pinchUpdateHook->m_original)(thisPtr, timeMs, delta, scale, rotation);
    }
}

void pinchEndHook(void *thisPtr, uint32_t timeMs, bool cancelled)
{
    auto *instance = dynamic_cast<HyprlandInputBackend *>(InputBackend::instance());
    if (!instance->touchpadPinchEnd(cancelled)) {
        (*(pinchEnd)instance->m_pinchEndHook->m_original)(thisPtr, timeMs, cancelled);
    }
}

HyprlandInputBackend::HyprlandInputBackend(Plugin *plugin)
{
    auto handle = plugin->handle();
    m_holdBeginHook = HyprlandAPI::createFunctionHook(handle, HyprlandAPI::findFunctionsByName(handle, "holdBegin")[0].address, (void *)&holdBeginHook);
    m_holdBeginHook->hook();
    m_holdEndHook = HyprlandAPI::createFunctionHook(handle, HyprlandAPI::findFunctionsByName(handle, "holdEnd")[0].address, (void *)&holdEndHook);
    m_holdEndHook->hook();
    m_pinchBeginHook = HyprlandAPI::createFunctionHook(handle, HyprlandAPI::findFunctionsByName(handle, "pinchBegin")[0].address, (void *)&pinchBeginHook);
    m_pinchBeginHook->hook();
    m_pinchUpdateHook = HyprlandAPI::createFunctionHook(handle, HyprlandAPI::findFunctionsByName(handle, "pinchUpdate")[0].address, (void *)&pinchUpdateHook);
    m_pinchUpdateHook->hook();
    m_pinchEndHook = HyprlandAPI::createFunctionHook(handle, HyprlandAPI::findFunctionsByName(handle, "pinchEnd")[0].address, (void *)&pinchEndHook);
    m_pinchEndHook->hook();

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
    checkDeviceChanges();
}

void HyprlandInputBackend::reset()
{
    for (auto &device : m_devices) {
        deviceRemoved(device.libinputactionsDevice.get());
    }
    m_devices.clear();
    m_hyprlandDevices.clear();
    LibinputCompositorInputBackend::reset();
}

void HyprlandInputBackend::checkDeviceChanges()
{
    auto &devices = g_pInputManager->m_hids;
    for (auto &device : devices) {
        if (std::ranges::any_of(m_hyprlandDevices, [&device](const auto &existingDevice) {
            return existingDevice == device.get();
        })) {
            continue;
        }

        HyprlandInputDevice newDevice{
            .hyprlandDevice = device.get(),
        };
        InputDeviceType type;
        QString name;
        if (auto *keyboard = dynamic_cast<IKeyboard *>(device.get())) {
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
                for (auto *hyprlandSignal : { &events.axis, &events.button, &events.motion, &events.holdBegin, &events.pinchBegin,
                                                        &events.swipeBegin }) {
                    newDevice.listeners.push_back(hyprlandSignal->registerListener([this, device](std::any) {
                        if (auto *foundDevice = findDevice(device.get())) {
                            m_currentPointingDevice = foundDevice->libinputactionsDevice.get();
                            m_currentTouchpad = m_currentPointingDevice;
                        }
                    }));
                }
            } else {
                for (auto *hyprlandSignal : { &events.axis, &events.button, &events.motion }) {
                    newDevice.listeners.push_back(hyprlandSignal->registerListener([this, device](std::any) {
                        if (auto *foundDevice = findDevice(device.get())) {
                            m_currentPointingDevice = foundDevice->libinputactionsDevice.get();
                        }
                    }));
                }
            }
        } else {
            continue;
        }

        newDevice.libinputactionsDevice = std::make_unique<InputDevice>(type, name);
        deviceAdded(newDevice.libinputactionsDevice.get());
        m_devices.push_back(std::move(newDevice));
        m_hyprlandDevices.push_back(device.get());
    }

    for (auto it = m_devices.begin(); it != m_devices.end();) {
        if (!std::ranges::any_of(m_hyprlandDevices, [it](const auto &device) {
            return device == it->hyprlandDevice;
        })) {
            deviceRemoved(it->libinputactionsDevice.get());
            std::erase(m_hyprlandDevices, it->hyprlandDevice);
            it = m_devices.erase(it);
            continue;
        }
        it++;
    }
}

void HyprlandInputBackend::keyboardKey(SCallbackInfo &info, const std::any &data)
{
    const auto map = std::any_cast<std::unordered_map<std::string, std::any>>(data);
    const auto event = std::any_cast<IKeyboard::SKeyEvent>(map.at("event"));
    const auto keyboard = std::any_cast<SP<IKeyboard>>(map.at("keyboard"));
    info.cancelled = LibinputCompositorInputBackend::keyboardKey(findInputActionsDevice(keyboard.get()), event.keycode,
        event.state == WL_KEYBOARD_KEY_STATE_PRESSED);
}

void HyprlandInputBackend::pointerAxis(SCallbackInfo &info, const std::any &data)
{
    const auto map = std::any_cast<std::unordered_map<std::string, std::any>>(data);
    const auto event = std::any_cast<IPointer::SAxisEvent>(map.at("event"));

    auto delta = event.axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL
        ? QPointF(event.delta, 0)
        : QPointF(0, event.delta);
    if (event.relativeDirection == WL_POINTER_AXIS_RELATIVE_DIRECTION_INVERTED) {
        delta *= -1;
    }
    info.cancelled = LibinputCompositorInputBackend::pointerAxis(m_currentPointingDevice, delta);
}

void HyprlandInputBackend::pointerButton(SCallbackInfo &info, const std::any &data)
{
    const auto event = std::any_cast<IPointer::SButtonEvent>(data);
    info.cancelled = LibinputCompositorInputBackend::pointerButton(m_currentPointingDevice, scanCodeToMouseButton(event.button), event.button,
        event.state == WL_POINTER_BUTTON_STATE_PRESSED);
}

void HyprlandInputBackend::pointerMotion(SCallbackInfo &info, const std::any &data)
{
    const auto pointerPosition = std::any_cast<const Vector2D>(data);
    const auto delta = pointerPosition - m_previousPointerPosition;
    m_previousPointerPosition = pointerPosition;
    info.cancelled = LibinputCompositorInputBackend::pointerMotion(m_currentPointingDevice, QPointF(delta.x, delta.y));
}

bool HyprlandInputBackend::touchpadHoldBegin(const uint8_t &fingers)
{
    return LibinputCompositorInputBackend::touchpadHoldBegin(m_currentTouchpad, fingers);
}

bool HyprlandInputBackend::touchpadHoldEnd(const bool &cancelled)
{
    return LibinputCompositorInputBackend::touchpadHoldEnd(m_currentTouchpad, cancelled);
}

bool HyprlandInputBackend::touchpadPinchBegin(const uint8_t &fingers)
{
   return LibinputCompositorInputBackend::touchpadPinchBegin(m_currentTouchpad, fingers);
}

bool HyprlandInputBackend::touchpadPinchUpdate(const double &scale, const double &angleDelta)
{
    return LibinputCompositorInputBackend::touchpadPinchUpdate(m_currentTouchpad, scale, angleDelta);
}

bool HyprlandInputBackend::touchpadPinchEnd(const bool &cancelled)
{
    return LibinputCompositorInputBackend::touchpadPinchEnd(m_currentTouchpad, cancelled);
}

void HyprlandInputBackend::touchpadSwipeBegin(SCallbackInfo &info, const std::any &data)
{
    const auto event = std::any_cast<IPointer::SSwipeBeginEvent>(data);
    info.cancelled = LibinputCompositorInputBackend::touchpadSwipeBegin(m_currentTouchpad, event.fingers);
}

void HyprlandInputBackend::touchpadSwipeUpdate(SCallbackInfo &info, const std::any &data)
{
    const auto event = std::any_cast<IPointer::SSwipeUpdateEvent>(data);
    info.cancelled = LibinputCompositorInputBackend::touchpadSwipeUpdate(m_currentTouchpad, QPointF(event.delta.x, event.delta.y));
}

void HyprlandInputBackend::touchpadSwipeEnd(SCallbackInfo &info, const std::any &data)
{
    const auto event = std::any_cast<IPointer::SSwipeEndEvent>(data);
    info.cancelled = LibinputCompositorInputBackend::touchpadSwipeEnd(m_currentTouchpad, event.cancelled);
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