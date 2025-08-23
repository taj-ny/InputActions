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
#include "globals.h"
#include "utils.h"
#include <libinputactions/input/events.h>
#include <libinputactions/interfaces/InputEmitter.h>
#include <libinputactions/triggers/StrokeTrigger.h>

#ifndef KWIN_6_3_OR_GREATER
#include "core/inputdevice.h"
#endif
#include "input_event.h"
#include "input_event_spy.h"

using namespace libinputactions;

KWinInputBackend::KWinInputBackend()
#ifdef KWIN_6_2_OR_GREATER
    : KWin::InputEventFilter(KWin::InputFilterOrder::TabBox)
#endif
{
    auto *input = KWin::input();
    connect(input, &KWin::InputRedirection::deviceAdded, this, &KWinInputBackend::kwinDeviceAdded);
    connect(input, &KWin::InputRedirection::deviceRemoved, this, &KWinInputBackend::kwinDeviceRemoved);

#ifdef KWIN_6_2_OR_GREATER
    KWin::input()->installInputEventFilter(this);
#else
    KWin::input()->prependInputEventFilter(this);
#endif
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

#ifdef KWIN_6_3_OR_GREATER
bool KWinInputBackend::pointerMotion(KWin::PointerMotionEvent *event)
{
    return LibinputCompositorInputBackend::pointerMotion(findInputActionsDevice(event->device), event->delta);
}

bool KWinInputBackend::pointerButton(KWin::PointerButtonEvent *event)
{
    return LibinputCompositorInputBackend::pointerButton(findInputActionsDevice(event->device),
                                                         event->button,
                                                         event->nativeButton,
                                                         event->state == PointerButtonStatePressed);
}

bool KWinInputBackend::keyboardKey(KWin::KeyboardKeyEvent *event)
{
    return LibinputCompositorInputBackend::keyboardKey(findInputActionsDevice(event->device), event->nativeScanCode, event->state == KeyboardKeyStatePressed);
}
#endif

#ifdef KWIN_6_3_OR_GREATER
bool KWinInputBackend::pointerAxis(KWin::PointerAxisEvent *event)
{
    const auto device = event->device;
    const auto eventDelta = event->delta;
    const auto orientation = event->orientation;
    const auto inverted = event->inverted;
#else
bool KWinInputBackend::wheelEvent(KWin::WheelEvent *event)
{
    const auto device = event->device();
    const auto eventDelta = event->delta();
    const auto orientation = event->orientation();
    const auto inverted = event->inverted();
#endif

    auto delta = orientation == Qt::Orientation::Horizontal ? QPointF(eventDelta, 0) : QPointF(0, eventDelta);
    if (inverted) {
        delta *= -1;
    }
    return LibinputCompositorInputBackend::pointerAxis(findInputActionsDevice(device), delta);
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