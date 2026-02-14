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

#include "HyprlandInputDevice.h"
#include "HyprlandInputBackend.h"
#include <hyprland/src/config/ConfigManager.hpp>
#include <hyprland/src/devices/IKeyboard.hpp>
#include <hyprland/src/devices/IPointer.hpp>
#undef HANDLE
#include <libinputactions/input/backends/InputBackend.h>

namespace InputActions
{

HyprlandInputDevice::HyprlandInputDevice(SP<IHID> device, InputDeviceType type, const std::string &name, HyprlandInputBackend *backend)
    : InputDevice(type, QString::fromStdString(name))
    , m_device(std::move(device))
    , m_backend(backend)
{
    if (g_pConfigManager->getDeviceString(name, "tap_button_map", "input:touchpad:tap_button_map") == "lmr") {
        properties().setTouchpadLmrTapButtonMap(true);
    }
}

HyprlandInputDevice::HyprlandInputDevice(SP<IPointer> device, InputDeviceType type, const std::string &name, HyprlandInputBackend *backend)
    : HyprlandInputDevice(dynamicPointerCast<IHID>(device), type, name, backend)
{
    m_listeners.push_back(device->m_pointerEvents.button.listen([this, backend, device = device.get()](const IPointer::SButtonEvent &event) {
        backend->onPointerButtonSignal(this, device, event);
    }));
    m_listeners.push_back(device->m_pointerEvents.holdBegin.listen([this, backend, device = device.get()](const IPointer::SHoldBeginEvent &event) {
        backend->onHoldBeginSignal(this, device, event);
    }));
    m_listeners.push_back(device->m_pointerEvents.holdEnd.listen([this, backend, device = device.get()](const IPointer::SHoldEndEvent &event) {
        backend->onHoldEndSignal(this, device, event);
    }));
    m_listeners.push_back(device->m_pointerEvents.pinchBegin.listen([this, backend, device = device.get()](const IPointer::SPinchBeginEvent &event) {
        backend->onPinchBeginSignal(this, device, event);
    }));
    m_listeners.push_back(device->m_pointerEvents.pinchUpdate.listen([this, backend, device = device.get()](const IPointer::SPinchUpdateEvent &event) {
        backend->onPinchUpdateSignal(this, device, event);
    }));
    m_listeners.push_back(device->m_pointerEvents.pinchEnd.listen([this, backend, device = device.get()](const IPointer::SPinchEndEvent &event) {
        backend->onPinchEndSignal(this, device, event);
    }));
    m_listeners.push_back(device->m_pointerEvents.swipeBegin.listen([this, backend, device = device.get()](const IPointer::SSwipeBeginEvent &event) {
        backend->onSwipeBeginSignal(this, device, event);
    }));
    m_listeners.push_back(device->m_pointerEvents.swipeUpdate.listen([this, backend, device = device.get()](const IPointer::SSwipeUpdateEvent &event) {
        backend->onSwipeUpdateSignal(this, device, event);
    }));
    m_listeners.push_back(device->m_pointerEvents.swipeEnd.listen([this, backend, device = device.get()](const IPointer::SSwipeEndEvent &event) {
        backend->onSwipeEndSignal(this, device, event);
    }));
}

HyprlandInputDevice::HyprlandInputDevice(SP<ITouch> device, InputDeviceType type, const std::string &name, HyprlandInputBackend *backend)
    : HyprlandInputDevice(dynamicPointerCast<IHID>(device), type, name, backend)
{
    const auto size = device->aq()->physicalSize;
    properties().setSize({size.x, size.y});

    m_listeners.push_back(device->m_touchEvents.cancel.listen([this, backend, device = device.get()](const ITouch::SCancelEvent &event) {
        backend->onTouchCancelSignal(this, device, event);
    }));
    m_listeners.push_back(device->m_touchEvents.frame.listen([this, backend, device = device.get()]() {
        backend->onTouchFrameSignal(this, device);
    }));
    m_listeners.push_back(device->m_touchEvents.motion.listen([this, backend, device = device.get()](const ITouch::SMotionEvent &event) {
        backend->onTouchMotionSignal(this, device, event);
    }));
    m_listeners.push_back(device->m_touchEvents.up.listen([this, backend, device = device.get()](const ITouch::SUpEvent &event) {
        backend->onTouchUpSignal(this, device, event);
    }));
}

std::unique_ptr<HyprlandInputDevice> HyprlandInputDevice::tryCreate(HyprlandInputBackend *backend, SP<IHID> device)
{
    if (auto keyboard = dynamicPointerCast<IKeyboard>(device)) {
        return std::unique_ptr<HyprlandInputDevice>(new HyprlandInputDevice(device, InputDeviceType::Keyboard, keyboard->m_deviceName, backend));
    }
    if (auto pointer = dynamicPointerCast<IPointer>(device)) {
        const auto type = pointer->m_isTouchpad ? InputDeviceType::Touchpad : InputDeviceType::Mouse;
        return std::unique_ptr<HyprlandInputDevice>(new HyprlandInputDevice(pointer, type, pointer->m_deviceName, backend));
    }
    if (auto touch = dynamicPointerCast<ITouch>(device)) {
        return std::unique_ptr<HyprlandInputDevice>(new HyprlandInputDevice(touch, InputDeviceType::Touchscreen, touch->m_deviceName, backend));
    }

    return {};
}

void HyprlandInputDevice::keyboardKey(KeyboardKey key, bool state)
{
    auto *keyboard = dynamic_cast<IKeyboard *>(m_device.get());
    if (!keyboard) {
        return;
    }

    auto aqKeyboard = keyboard->aq();
    if (!aqKeyboard) {
        return;
    }

    g_inputBackend->setIgnoreEvents(true);
    keyboard->aq()->events.key.emit(Aquamarine::IKeyboard::SKeyEvent{
        .key = key.scanCode(),
        .pressed = state,
    });
    InputDevice::keyboardKey(key, state);

    uint32_t modifiers{};
    for (const auto key : virtualState().pressedKeys()) {
        if (const auto modifier = g_pKeybindManager->keycodeToModifier(key.scanCode() + 8)) {
            modifiers |= modifier;
        }
    }

    if (const auto modifier = g_pKeybindManager->keycodeToModifier(key.scanCode() + 8)) {
        if (state) {
            modifiers |= modifier;
        } else {
            modifiers &= ~modifier;
        }
        keyboard->aq()->events.modifiers.emit(Aquamarine::IKeyboard::SModifiersEvent{
            .depressed = modifiers,
        });
    }

    g_inputBackend->setIgnoreEvents(false);
}

void HyprlandInputDevice::mouseButton(uint32_t button, bool state)
{
    auto *pointer = dynamic_cast<IPointer *>(m_device.get());
    if (!pointer) {
        return;
    }

    g_inputBackend->setIgnoreEvents(true);
    pointer->m_pointerEvents.button.emit(IPointer::SButtonEvent{
        .button = button,
        .state = state ? WL_POINTER_BUTTON_STATE_PRESSED : WL_POINTER_BUTTON_STATE_RELEASED,
        .mouse = true,
    });
    pointer->m_pointerEvents.frame.emit();
    g_inputBackend->setIgnoreEvents(false);
}

void HyprlandInputDevice::resetVirtualDeviceState()
{
    if (type() != InputDeviceType::Touchscreen) {
        return;
    }
    auto *touchscreen = dynamic_cast<ITouch *>(m_device.get());

    m_backend->setIgnoreEvents(true);

    for (const auto *point : physicalState().validTouchPoints()) {
        touchscreen->m_touchEvents.up.emit(ITouch::SUpEvent{
            .touchID = point->id,
        });
    }
    touchscreen->m_touchEvents.frame.emit();

    m_backend->setIgnoreEvents(false);
}

void HyprlandInputDevice::restoreVirtualDeviceState()
{
    if (type() != InputDeviceType::Touchscreen) {
        return;
    }
    auto touchscreen = dynamicPointerCast<ITouch>(m_device);

    m_backend->setIgnoreEvents(true);

    for (const auto *point : physicalState().validTouchPoints()) {
        touchscreen->m_touchEvents.down.emit(ITouch::SDownEvent{
            .touchID = point->id,
            .pos = {point->rawInitialPosition.x(), point->rawInitialPosition.y()},
            .device = touchscreen,
        });
    }
    touchscreen->m_touchEvents.frame.emit();

    for (const auto *point : physicalState().validTouchPoints()) {
        touchscreen->m_touchEvents.motion.emit(ITouch::SMotionEvent{
            .touchID = point->id,
            .pos = {point->rawPosition.x(), point->rawPosition.y()},
        });
    }
    touchscreen->m_touchEvents.frame.emit();

    m_backend->setIgnoreEvents(false);
}

void HyprlandInputDevice::touchscreenTapDown(const std::vector<QPointF> &points)
{
    auto touchscreen = dynamicPointerCast<ITouch>(m_device);

    m_backend->setIgnoreEvents(true);

    for (size_t i = 0; i < points.size(); ++i) {
        const auto &point = points[i];
        touchscreen->m_touchEvents.down.emit(ITouch::SDownEvent{
            .touchID = static_cast<int32_t>(i),
            .pos = {point.x(), point.y()},
            .device = touchscreen,
        });
    }
    touchscreen->m_touchEvents.frame.emit();

    m_backend->setIgnoreEvents(false);
}

void HyprlandInputDevice::touchscreenTapUp(const std::vector<QPointF> &points)
{
    auto *touchscreen = dynamic_cast<ITouch *>(m_device.get());

    m_backend->setIgnoreEvents(true);

    for (size_t i = 0; i < points.size(); ++i) {
        touchscreen->m_touchEvents.up.emit(ITouch::SUpEvent{
            .touchID = static_cast<int32_t>(i),
        });
    }
    touchscreen->m_touchEvents.frame.emit();

    m_backend->setIgnoreEvents(false);
}

}