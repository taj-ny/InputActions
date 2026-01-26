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

namespace InputActions
{

HyprlandInputBackend::HyprlandInputBackend(void *handle)
    : m_holdBeginHook(handle, "holdBegin")
    , m_holdEndHook(handle, "holdEnd")
    , m_pinchBeginHook(handle, "pinchBegin")
    , m_pinchUpdateHook(handle, "pinchUpdate")
    , m_pinchEndHook(handle, "pinchEnd")
    , m_pointerAxisHook(handle, "onMouseWheel")
    , m_pointerButtonHook(handle, "onMouseButton")
    , m_pointerMotionHook(handle, "onMouseMoved")
    , m_swipeBeginHook(handle, "swipeBegin")
    , m_swipeUpdateHook(handle, "swipeUpdate")
    , m_swipeEndHook(handle, "swipeEnd")
{
    m_events.push_back(HyprlandAPI::registerCallbackDynamic(handle, "keyPress", [this](void *, SCallbackInfo &info, std::any data) {
        keyboardKey(info, data);
    }));

    connect(&m_deviceChangeTimer, &QTimer::timeout, this, &HyprlandInputBackend::checkDeviceChanges);
    m_deviceChangeTimer.setInterval(1000);
}

HyprlandInputBackend::~HyprlandInputBackend()
{
    reset();
}

void HyprlandInputBackend::initialize()
{
    LibinputInputBackend::initialize();

    m_blockHookCalls = true;
    m_deviceChangeTimer.start();
    checkDeviceChanges();
}

void HyprlandInputBackend::reset()
{
    for (const auto &device : m_devices) {
        deviceRemoved(device);
    }
    m_devices.clear();
    m_previousHids.clear();
    m_deviceChangeTimer.stop();
    m_blockHookCalls = false;
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

            newDevice.listeners.push_back(pointer->m_pointerEvents.button.listen([this, pointer](const IPointer::SButtonEvent &event) {
                onPointerButtonSignal(pointer, event);
            }));
            newDevice.listeners.push_back(pointer->m_pointerEvents.holdBegin.listen([this, pointer](const IPointer::SHoldBeginEvent &event) {
                onHoldBeginSignal(pointer, event);
            }));
            newDevice.listeners.push_back(pointer->m_pointerEvents.holdEnd.listen([this, pointer](const IPointer::SHoldEndEvent &event) {
                onHoldEndSignal(pointer, event);
            }));
            newDevice.listeners.push_back(pointer->m_pointerEvents.pinchBegin.listen([this, pointer](const IPointer::SPinchBeginEvent &event) {
                onPinchBeginSignal(pointer, event);
            }));
            newDevice.listeners.push_back(pointer->m_pointerEvents.pinchUpdate.listen([this, pointer](const IPointer::SPinchUpdateEvent &event) {
                onPinchUpdateSignal(pointer, event);
            }));
            newDevice.listeners.push_back(pointer->m_pointerEvents.pinchEnd.listen([this, pointer](const IPointer::SPinchEndEvent &event) {
                onPinchEndSignal(pointer, event);
            }));
            newDevice.listeners.push_back(pointer->m_pointerEvents.swipeBegin.listen([this, pointer](const IPointer::SSwipeBeginEvent &event) {
                onSwipeBeginSignal(pointer, event);
            }));
            newDevice.listeners.push_back(pointer->m_pointerEvents.swipeUpdate.listen([this, pointer](const IPointer::SSwipeUpdateEvent &event) {
                onSwipeUpdateSignal(pointer, event);
            }));
            newDevice.listeners.push_back(pointer->m_pointerEvents.swipeEnd.listen([this, pointer](const IPointer::SSwipeEndEvent &event) {
                onSwipeEndSignal(pointer, event);
            }));
        } else {
            continue;
        }

        newDevice.libinputactionsDevice = std::make_unique<InputDevice>(type, name);
        if (g_pConfigManager->getDeviceString(device->m_deviceName, "tap_button_map", "input:touchpad:tap_button_map") == "lmr") {
            newDevice.libinputactionsDevice->properties().setLmrTapButtonMap(true);
        }

        LibevdevComplementaryInputBackend::addDevice(newDevice.libinputactionsDevice.get());
        InputBackend::addDevice(newDevice.libinputactionsDevice.get());
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
    removeDevice(device.libinputactionsDevice.get());
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

void HyprlandInputBackend::pointerAxisHook(void *thisPtr, IPointer::SAxisEvent event, SP<IPointer> sender)
{
    auto *self = dynamic_cast<HyprlandInputBackend *>(g_inputBackend.get());
    if (self->m_ignoreEvents || !self->m_blockHookCalls) {
        self->m_pointerAxisHook(thisPtr, event, sender);
        return;
    }

    auto delta = event.axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL ? QPointF(event.delta, 0) : QPointF(0, event.delta);
    if (event.relativeDirection == WL_POINTER_AXIS_RELATIVE_DIRECTION_INVERTED) {
        delta *= -1;
    }

    if (!self->pointerAxis(self->findInputActionsDevice(sender.get()), delta, true)) {
        self->m_pointerAxisHook(thisPtr, event, sender);
    }
}

void HyprlandInputBackend::pointerMotionHook(void *thisPtr, IPointer::SMotionEvent event)
{
    auto *self = dynamic_cast<HyprlandInputBackend *>(g_inputBackend.get());
    if (self->m_ignoreEvents || !self->m_blockHookCalls) {
        self->m_pointerMotionHook(thisPtr, event);
        return;
    }

    if (!self->pointerMotion(self->findInputActionsDevice(event.device.get()), {{event.delta.x, event.delta.y}, {event.unaccel.x, event.unaccel.y}})) {
        self->m_pointerMotionHook(thisPtr, event);
    }
}

void HyprlandInputBackend::onHoldBeginSignal(IPointer *sender, const IPointer::SHoldBeginEvent &event)
{
    if (m_ignoreEvents) {
        return;
    }

    if (!touchpadHoldBegin(findInputActionsDevice(sender), event.fingers)) {
        m_ignoreEvents = true;
        sender->m_pointerEvents.holdBegin.emit(event);
        m_ignoreEvents = false;
    }
}

void HyprlandInputBackend::holdBeginHook(void *thisPtr, uint32_t timeMs, uint32_t fingers)
{
    auto *self = dynamic_cast<HyprlandInputBackend *>(g_inputBackend.get());
    if (self->m_ignoreEvents || !self->m_blockHookCalls) {
        self->m_holdBeginHook(thisPtr, timeMs, fingers);
    }
}

void HyprlandInputBackend::onHoldEndSignal(IPointer *sender, const IPointer::SHoldEndEvent &event)
{
    if (m_ignoreEvents) {
        return;
    }

    if (!touchpadHoldEnd(findInputActionsDevice(sender), event.cancelled)) {
        m_ignoreEvents = true;
        sender->m_pointerEvents.holdEnd.emit(event);
        m_ignoreEvents = false;
    }
}

void HyprlandInputBackend::holdEndHook(void *thisPtr, uint32_t timeMs, bool cancelled)
{
    auto *self = dynamic_cast<HyprlandInputBackend *>(g_inputBackend.get());
    if (self->m_ignoreEvents || !self->m_blockHookCalls) {
        self->m_holdEndHook(thisPtr, timeMs, cancelled);
    }
}

void HyprlandInputBackend::onPinchBeginSignal(IPointer *sender, const IPointer::SPinchBeginEvent &event)
{
    if (m_ignoreEvents) {
        return;
    }

    if (!touchpadPinchBegin(findInputActionsDevice(sender), event.fingers)) {
        m_ignoreEvents = true;
        sender->m_pointerEvents.pinchBegin.emit(event);
        m_ignoreEvents = false;
    }
}

void HyprlandInputBackend::pinchBeginHook(void *thisPtr, uint32_t timeMs, uint32_t fingers)
{
    auto *self = dynamic_cast<HyprlandInputBackend *>(g_inputBackend.get());
    if (self->m_ignoreEvents || !self->m_blockHookCalls) {
        self->m_pinchBeginHook(thisPtr, timeMs, fingers);
    }
}

void HyprlandInputBackend::onPinchUpdateSignal(IPointer *sender, const IPointer::SPinchUpdateEvent &event)
{
    if (m_ignoreEvents) {
        return;
    }

    if (!touchpadPinchUpdate(findInputActionsDevice(sender), event.scale, event.rotation)) {
        m_ignoreEvents = true;
        sender->m_pointerEvents.pinchUpdate.emit(event);
        m_ignoreEvents = false;
    }
}

void HyprlandInputBackend::pinchUpdateHook(void *thisPtr, uint32_t timeMs, const Vector2D &delta, double scale, double rotation)
{
    auto *self = dynamic_cast<HyprlandInputBackend *>(g_inputBackend.get());
    if (self->m_ignoreEvents || !self->m_blockHookCalls) {
        self->m_pinchUpdateHook(thisPtr, timeMs, delta, scale, rotation);
    }
}

void HyprlandInputBackend::onPinchEndSignal(IPointer *sender, const IPointer::SPinchEndEvent &event)
{
    if (m_ignoreEvents) {
        return;
    }

    if (!touchpadPinchEnd(findInputActionsDevice(sender), event.cancelled)) {
        m_ignoreEvents = true;
        sender->m_pointerEvents.pinchEnd.emit(event);
        m_ignoreEvents = false;
    }
}

void HyprlandInputBackend::pinchEndHook(void *thisPtr, uint32_t timeMs, bool cancelled)
{
    auto *self = dynamic_cast<HyprlandInputBackend *>(g_inputBackend.get());
    if (self->m_ignoreEvents || !self->m_blockHookCalls) {
        self->m_pinchEndHook(thisPtr, timeMs, cancelled);
    }
}

void HyprlandInputBackend::onPointerButtonSignal(IPointer *sender, const IPointer::SButtonEvent &event)
{
    if (m_ignoreEvents) {
        return;
    }

    if (!pointerButton(findInputActionsDevice(sender), scanCodeToMouseButton(event.button), event.button, event.state == WL_POINTER_BUTTON_STATE_PRESSED)) {
        m_ignoreEvents = true;
        sender->m_pointerEvents.button.emit(event);
        m_ignoreEvents = false;
    }
}

void HyprlandInputBackend::pointerButtonHook(void *thisPtr, IPointer::SButtonEvent event)
{
    auto *self = dynamic_cast<HyprlandInputBackend *>(g_inputBackend.get());
    if (self->m_ignoreEvents || !self->m_blockHookCalls) {
        self->m_pointerButtonHook(thisPtr, event);
    }
}

void HyprlandInputBackend::onSwipeBeginSignal(IPointer *sender, const IPointer::SSwipeBeginEvent &event)
{
    if (m_ignoreEvents) {
        return;
    }

    if (!touchpadSwipeBegin(findInputActionsDevice(sender), event.fingers)) {
        m_ignoreEvents = true;
        sender->m_pointerEvents.swipeBegin.emit(event);
        m_ignoreEvents = false;
    }
}

void HyprlandInputBackend::swipeBeginHook(void *thisPtr, uint32_t timeMs, uint32_t fingers)
{
    auto *self = dynamic_cast<HyprlandInputBackend *>(g_inputBackend.get());
    if (self->m_ignoreEvents || !self->m_blockHookCalls) {
        self->m_swipeBeginHook(thisPtr, timeMs, fingers);
    }
}

void HyprlandInputBackend::onSwipeUpdateSignal(IPointer *sender, const IPointer::SSwipeUpdateEvent &event)
{
    if (m_ignoreEvents) {
        return;
    }

    if (!touchpadSwipeUpdate(findInputActionsDevice(sender), {{event.delta.x, event.delta.y}})) {
        m_ignoreEvents = true;
        sender->m_pointerEvents.swipeUpdate.emit(event);
        m_ignoreEvents = false;
    }
}

void HyprlandInputBackend::swipeUpdateHook(void *thisPtr, uint32_t timeMs, const Vector2D &delta)
{
    auto *self = dynamic_cast<HyprlandInputBackend *>(g_inputBackend.get());
    if (self->m_ignoreEvents || !self->m_blockHookCalls) {
        self->m_swipeUpdateHook(thisPtr, timeMs, delta);
    }
}

void HyprlandInputBackend::onSwipeEndSignal(IPointer *sender, const IPointer::SSwipeEndEvent &event)
{
    if (m_ignoreEvents) {
        return;
    }

    if (!touchpadSwipeEnd(findInputActionsDevice(sender), event.cancelled)) {
        m_ignoreEvents = true;
        sender->m_pointerEvents.swipeEnd.emit(event);
        m_ignoreEvents = false;
    }
}

void HyprlandInputBackend::swipeEndHook(void *thisPtr, uint32_t timeMs, bool cancelled)
{
    auto *self = dynamic_cast<HyprlandInputBackend *>(g_inputBackend.get());
    if (self->m_ignoreEvents || !self->m_blockHookCalls) {
        self->m_swipeEndHook(thisPtr, timeMs, cancelled);
    }
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

}