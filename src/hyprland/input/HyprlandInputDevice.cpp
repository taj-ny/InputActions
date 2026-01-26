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
#include "interfaces/HyprlandInputEmitter.h"
#include <hyprland/src/config/ConfigManager.hpp>
#undef HANDLE

namespace InputActions
{

HyprlandInputDevice::HyprlandInputDevice(IHID *device, InputDeviceType type, const std::string &name)
    : InputDevice(type, QString::fromStdString(name))
    , m_hyprlandDevice(device)
{
    if (g_pConfigManager->getDeviceString(name, "tap_button_map", "input:touchpad:tap_button_map") == "lmr") {
        properties().setLmrTapButtonMap(true);
    }
}

HyprlandInputDevice::HyprlandInputDevice(IPointer *device, InputDeviceType type, const std::string &name, HyprlandInputBackend *backend)
    : HyprlandInputDevice(static_cast<IHID *>(device), type, name)
{
    m_listeners.push_back(device->m_pointerEvents.button.listen([this, backend, device](const IPointer::SButtonEvent &event) {
        backend->onPointerButtonSignal(this, device, event);
    }));
    m_listeners.push_back(device->m_pointerEvents.holdBegin.listen([this, backend, device](const IPointer::SHoldBeginEvent &event) {
        backend->onHoldBeginSignal(this, device, event);
    }));
    m_listeners.push_back(device->m_pointerEvents.holdEnd.listen([this, backend, device](const IPointer::SHoldEndEvent &event) {
        backend->onHoldEndSignal(this, device, event);
    }));
    m_listeners.push_back(device->m_pointerEvents.pinchBegin.listen([this, backend, device](const IPointer::SPinchBeginEvent &event) {
        backend->onPinchBeginSignal(this, device, event);
    }));
    m_listeners.push_back(device->m_pointerEvents.pinchUpdate.listen([this, backend, device](const IPointer::SPinchUpdateEvent &event) {
        backend->onPinchUpdateSignal(this, device, event);
    }));
    m_listeners.push_back(device->m_pointerEvents.pinchEnd.listen([this, backend, device](const IPointer::SPinchEndEvent &event) {
        backend->onPinchEndSignal(this, device, event);
    }));
    m_listeners.push_back(device->m_pointerEvents.swipeBegin.listen([this, backend, device](const IPointer::SSwipeBeginEvent &event) {
        backend->onSwipeBeginSignal(this, device, event);
    }));
    m_listeners.push_back(device->m_pointerEvents.swipeUpdate.listen([this, backend, device](const IPointer::SSwipeUpdateEvent &event) {
        backend->onSwipeUpdateSignal(this, device, event);
    }));
    m_listeners.push_back(device->m_pointerEvents.swipeEnd.listen([this, backend, device](const IPointer::SSwipeEndEvent &event) {
        backend->onSwipeEndSignal(this, device, event);
    }));
}

std::unique_ptr<HyprlandInputDevice> HyprlandInputDevice::tryCreate(HyprlandInputBackend *backend, IHID *device)
{
    if (auto *keyboard = dynamic_cast<IKeyboard *>(device)) {
        if (keyboard->aq().get() == std::dynamic_pointer_cast<HyprlandInputEmitter>(g_inputEmitter)->keyboard()) {
            return {};
        }

        return std::unique_ptr<HyprlandInputDevice>(new HyprlandInputDevice(device, InputDeviceType::Keyboard, keyboard->m_deviceName));
    }
    if (auto *pointer = dynamic_cast<IPointer *>(device)) {
        const auto type = pointer->m_isTouchpad ? InputDeviceType::Touchpad : InputDeviceType::Mouse;
        return std::unique_ptr<HyprlandInputDevice>(new HyprlandInputDevice(pointer, type, pointer->m_deviceName, backend));
    }

    return {};
}

}