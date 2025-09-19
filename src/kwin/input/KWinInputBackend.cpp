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

#include "KWinInputBackend.h"
#include "input_event.h"
#include <libinputactions/input/events.h>
#include <libinputactions/interfaces/InputEmitter.h>
#include <libinputactions/triggers/StrokeTrigger.h>

using namespace libinputactions;

KWinInputBackend::KWinInputBackend()
    : InputEventFilter(KWin::InputFilterOrder::TabBox)
{
    auto *input = KWin::input();
    connect(input, &KWin::InputRedirection::deviceAdded, this, &KWinInputBackend::kwinDeviceAdded);
    connect(input, &KWin::InputRedirection::deviceRemoved, this, &KWinInputBackend::kwinDeviceRemoved);

    input->installInputEventFilter(this);
}

KWinInputBackend::~KWinInputBackend()
{
    reset();
    if (auto *input = KWin::input()) {
        input->uninstallInputEventFilter(this);
    }
}

void KWinInputBackend::initialize()
{
    for (auto *device : KWin::input()->devices()) {
        kwinDeviceAdded(device);
    }
}

void KWinInputBackend::reset()
{
    for (auto &device : m_devices) {
        deviceRemoved(device.libinputactionsDevice.get());
    }
    m_devices.clear();
    LibinputCompositorInputBackend::reset();
}

bool KWinInputBackend::holdGestureBegin(int fingerCount, std::chrono::microseconds time)
{
    return touchpadHoldBegin(currentTouchpad(), fingerCount);
}

bool KWinInputBackend::holdGestureEnd(std::chrono::microseconds time)
{
    return touchpadHoldEnd(currentTouchpad(), false);
}

bool KWinInputBackend::holdGestureCancelled(std::chrono::microseconds time)
{
    return touchpadHoldEnd(currentTouchpad(), true);
}

bool KWinInputBackend::swipeGestureBegin(int fingerCount, std::chrono::microseconds time)
{
    return touchpadSwipeBegin(currentTouchpad(), fingerCount);
}

bool KWinInputBackend::swipeGestureUpdate(const QPointF &delta, std::chrono::microseconds time)
{
    return touchpadSwipeUpdate(currentTouchpad(), delta);
}

bool KWinInputBackend::swipeGestureEnd(std::chrono::microseconds time)
{
    return touchpadSwipeEnd(currentTouchpad(), false);
}

bool KWinInputBackend::swipeGestureCancelled(std::chrono::microseconds time)
{
    return touchpadSwipeEnd(currentTouchpad(), true);
}

bool KWinInputBackend::pinchGestureBegin(int fingerCount, std::chrono::microseconds time)
{
    return touchpadPinchBegin(currentTouchpad(), fingerCount);
}

bool KWinInputBackend::pinchGestureUpdate(qreal scale, qreal angleDelta, const QPointF &delta, std::chrono::microseconds time)
{
    return touchpadPinchUpdate(currentTouchpad(), scale, angleDelta);
}

bool KWinInputBackend::pinchGestureEnd(std::chrono::microseconds time)
{
    return touchpadPinchEnd(currentTouchpad(), false);
}

bool KWinInputBackend::pinchGestureCancelled(std::chrono::microseconds time)
{
    return touchpadPinchEnd(currentTouchpad(), true);
}

bool KWinInputBackend::pointerAxis(KWin::PointerAxisEvent *event)
{
    auto delta = event->orientation == Qt::Orientation::Horizontal ? QPointF(event->delta, 0) : QPointF(0, event->delta);
    if (event->inverted) {
        delta *= -1;
    }
    return LibinputCompositorInputBackend::pointerAxis(findInputActionsDevice(event->device), delta);
}

bool KWinInputBackend::pointerButton(KWin::PointerButtonEvent *event)
{
    return LibinputCompositorInputBackend::pointerButton(findInputActionsDevice(event->device),
                                                         event->button,
                                                         event->nativeButton,
                                                         event->state == KWin::PointerButtonState::Pressed);
}

bool KWinInputBackend::pointerMotion(KWin::PointerMotionEvent *event)
{
    return LibinputCompositorInputBackend::pointerMotion(findInputActionsDevice(event->device), event->delta, event->deltaUnaccelerated);
}

bool KWinInputBackend::keyboardKey(KWin::KeyboardKeyEvent *event)
{
    return LibinputCompositorInputBackend::keyboardKey(findInputActionsDevice(event->device),
                                                       event->nativeScanCode,
                                                       event->state == KWin::KeyboardKeyState::Pressed);
}

void KWinInputBackend::kwinDeviceAdded(KWin::InputDevice *kwinDevice)
{
    InputDeviceType type;
    if (isMouse(kwinDevice)) {
        type = InputDeviceType::Mouse;
    } else if (kwinDevice->isKeyboard()) {
        type = InputDeviceType::Keyboard;
    } else if (kwinDevice->isTouchpad()) {
        type = InputDeviceType::Touchpad;
    } else {
        return;
    }

    KWinInputDevice device{
        .kwinDevice = kwinDevice,
        .libinputactionsDevice = std::make_unique<libinputactions::InputDevice>(type, kwinDevice->name(), kwinDevice->property("sysName").toString()),
    };
    if (kwinDevice->property("lmrTapButtonMap").value<bool>()) {
        device.libinputactionsDevice->properties().setLmrTapButtonMap(true);
    }
    deviceAdded(device.libinputactionsDevice.get());
    m_devices.push_back(std::move(device));
}

void KWinInputBackend::kwinDeviceRemoved(const KWin::InputDevice *kwinDevice)
{
    for (auto it = m_devices.begin(); it != m_devices.end(); it++) {
        if (it->kwinDevice == kwinDevice) {
            deviceRemoved(it->libinputactionsDevice.get());
            m_devices.erase(it);
            return;
        }
    }
}

libinputactions::InputDevice *KWinInputBackend::findInputActionsDevice(const KWin::InputDevice *kwinDevice)
{
    for (auto &device : m_devices) {
        if (device.kwinDevice == kwinDevice) {
            return device.libinputactionsDevice.get();
        }
    }
    return {};
}

libinputactions::InputDevice *KWinInputBackend::currentTouchpad()
{
    for (const auto *device : KWin::input()->devices()) {
        if (device->isTouchpad()) {
            return findInputActionsDevice(device);
        }
    }
    return nullptr;
}

bool KWinInputBackend::isMouse(const KWin::InputDevice *device) const
{
    return device->isPointer() && !device->isTouch() && !device->isTouchpad();
}