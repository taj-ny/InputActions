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
#include "KWinInputDevice.h"
#include "core/output.h"
#include "input_event.h"
#include "interfaces/KWinInputEmitter.h"
#include "utils.h"
#include "workspace.h"
#include <libinputactions/input/events.h>
#include <libinputactions/interfaces/InputEmitter.h>
#include <libinputactions/triggers/StrokeTrigger.h>

namespace InputActions
{

KWinInputBackend::KWinInputBackend()
    : InputEventFilter(KWin::InputFilterOrder::ScreenEdge)
    , m_input(KWin::input())
{
    m_input->installInputEventFilter(this);
    m_input->installInputEventSpy(&m_keyboardModifierSpy);
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
    LibinputInputBackend::initialize();

    connect(m_input, &KWin::InputRedirection::deviceAdded, this, &KWinInputBackend::kwinDeviceAdded);
    connect(m_input, &KWin::InputRedirection::deviceRemoved, this, &KWinInputBackend::kwinDeviceRemoved);
    for (auto *device : KWin::input()->devices()) {
        kwinDeviceAdded(device);
    }
}

void KWinInputBackend::reset()
{
    disconnect(m_input, nullptr, this, nullptr);
    for (auto &device : m_devices) {
        removeDevice(device.get());
    }
    m_devices.clear();
    LibinputInputBackend::reset();
}

#ifdef KWIN_6_5_OR_GREATER
bool KWinInputBackend::holdGestureBegin(KWin::PointerHoldGestureBeginEvent *event)
{
    const auto fingerCount = event->fingerCount;
#else
bool KWinInputBackend::holdGestureBegin(int fingerCount, std::chrono::microseconds time)
{
#endif
    return touchpadHoldBegin(firstTouchpad(), fingerCount);
}

#ifdef KWIN_6_5_OR_GREATER
bool KWinInputBackend::holdGestureEnd(KWin::PointerHoldGestureEndEvent *event)
#else
bool KWinInputBackend::holdGestureEnd(std::chrono::microseconds time)
#endif
{
    return touchpadHoldEnd(firstTouchpad(), false);
}

#ifdef KWIN_6_5_OR_GREATER
bool KWinInputBackend::holdGestureCancelled(KWin::PointerHoldGestureCancelEvent *event)
#else
bool KWinInputBackend::holdGestureCancelled(std::chrono::microseconds time)
#endif
{
    return touchpadHoldEnd(firstTouchpad(), true);
}

#ifdef KWIN_6_5_OR_GREATER
bool KWinInputBackend::swipeGestureBegin(KWin::PointerSwipeGestureBeginEvent *event)
{
    const auto fingerCount = event->fingerCount;
#else
bool KWinInputBackend::swipeGestureBegin(int fingerCount, std::chrono::microseconds time)
{
#endif
    return touchpadSwipeBegin(firstTouchpad(), fingerCount);
}

#ifdef KWIN_6_5_OR_GREATER
bool KWinInputBackend::swipeGestureUpdate(KWin::PointerSwipeGestureUpdateEvent *event)
{
    const auto &delta = event->delta;
#else
bool KWinInputBackend::swipeGestureUpdate(const QPointF &delta, std::chrono::microseconds time)
{
#endif
    return touchpadSwipeUpdate(firstTouchpad(), {delta});
}

#ifdef KWIN_6_5_OR_GREATER
bool KWinInputBackend::swipeGestureEnd(KWin::PointerSwipeGestureEndEvent *event)
#else
bool KWinInputBackend::swipeGestureEnd(std::chrono::microseconds time)
#endif
{
    return touchpadSwipeEnd(firstTouchpad(), false);
}

#ifdef KWIN_6_5_OR_GREATER
bool KWinInputBackend::swipeGestureCancelled(KWin::PointerSwipeGestureCancelEvent *event)
#else
bool KWinInputBackend::swipeGestureCancelled(std::chrono::microseconds time)
#endif
{
    return touchpadSwipeEnd(firstTouchpad(), true);
}

#ifdef KWIN_6_5_OR_GREATER
bool KWinInputBackend::pinchGestureBegin(KWin::PointerPinchGestureBeginEvent *event)
{
    const auto fingerCount = event->fingerCount;
#else
bool KWinInputBackend::pinchGestureBegin(int fingerCount, std::chrono::microseconds time)
{
#endif
    return touchpadPinchBegin(firstTouchpad(), fingerCount);
}

#ifdef KWIN_6_5_OR_GREATER
bool KWinInputBackend::pinchGestureUpdate(KWin::PointerPinchGestureUpdateEvent *event)
{
    const auto scale = event->scale;
    const auto angleDelta = event->angleDelta;
#else
bool KWinInputBackend::pinchGestureUpdate(qreal scale, qreal angleDelta, const QPointF &delta, std::chrono::microseconds time)
{
#endif
    return touchpadPinchUpdate(firstTouchpad(), scale, angleDelta);
}

#ifdef KWIN_6_5_OR_GREATER
bool KWinInputBackend::pinchGestureEnd(KWin::PointerPinchGestureEndEvent *event)
#else
bool KWinInputBackend::pinchGestureEnd(std::chrono::microseconds time)
#endif
{
    return touchpadPinchEnd(firstTouchpad(), false);
}

#ifdef KWIN_6_5_OR_GREATER
bool KWinInputBackend::pinchGestureCancelled(KWin::PointerPinchGestureCancelEvent *event)
#else
bool KWinInputBackend::pinchGestureCancelled(std::chrono::microseconds time)
#endif
{
    return touchpadPinchEnd(firstTouchpad(), true);
}

bool KWinInputBackend::pointerAxis(KWin::PointerAxisEvent *event)
{
    auto delta = event->orientation == Qt::Orientation::Horizontal ? QPointF(event->delta, 0) : QPointF(0, event->delta);
    if (event->inverted) {
        delta *= -1;
    }
    return LibinputInputBackend::pointerAxis(findDevice(event->device), delta, true);
}

bool KWinInputBackend::pointerButton(KWin::PointerButtonEvent *event)
{
    return LibinputInputBackend::pointerButton(findDevice(event->device),
                                               event->button,
                                               event->nativeButton,
                                               event->state == KWin::PointerButtonState::Pressed);
}

bool KWinInputBackend::pointerMotion(KWin::PointerMotionEvent *event)
{
    return LibinputInputBackend::pointerMotion(findDevice(event->device), {event->delta, event->deltaUnaccelerated});
}

bool KWinInputBackend::keyboardKey(KWin::KeyboardKeyEvent *event)
{
    return LibinputInputBackend::keyboardKey(findDevice(event->device), event->nativeScanCode, event->state == KWin::KeyboardKeyState::Pressed);
}

#ifdef KWIN_6_5_OR_GREATER
bool KWinInputBackend::touchDown(KWin::TouchDownEvent *event)
{
    auto *sender = firstTouchscreen();
    if (!sender) {
        return false;
    }

    auto *output = KWin::workspace()->outputAt(event->pos);
    const QPointF position((event->pos.x() - output->geometry().x()) / output->geometry().width() * sender->properties().size().width(),
                           (event->pos.y() - output->geometry().y()) / output->geometry().height() * sender->properties().size().height());
    return touchscreenTouchDown(sender, event->id, position, event->pos);
}

bool KWinInputBackend::touchMotion(KWin::TouchMotionEvent *event)
{
    auto *sender = firstTouchscreen();
    if (!sender) {
        return false;
    }

    auto *output = KWin::workspace()->outputAt(event->pos);
    const QPointF position((event->pos.x() - output->geometry().x()) / output->geometry().width() * sender->properties().size().width(),
                           (event->pos.y() - output->geometry().y()) / output->geometry().height() * sender->properties().size().height());
    return touchscreenTouchMotion(sender, event->id, position, event->pos);
}

bool KWinInputBackend::touchUp(KWin::TouchUpEvent *event)
{
    return touchscreenTouchUp(firstTouchscreen(), event->id);
}

bool KWinInputBackend::touchCancel()
{
    return touchscreenTouchCancel(firstTouchscreen());
}

bool KWinInputBackend::touchFrame()
{
    return touchscreenTouchFrame(firstTouchscreen());
}
#endif

void KWinInputBackend::touchpadPinchBlockingStopped(uint32_t fingers)
{
    m_ignoreEvents = true;
    const auto time = timestamp();
#ifdef KWIN_6_5_OR_GREATER
    KWin::PointerPinchGestureBeginEvent event{
        .fingerCount = static_cast<int>(fingers),
        .time = time,
    };
    m_input->processSpies(&KWin::InputEventSpy::pinchGestureBegin, &event);
    m_input->processFilters(&KWin::InputEventFilter::pinchGestureBegin, &event);
#else
    m_input->processSpies([&fingers, &time](auto &&spy) {
        spy->pinchGestureBegin(fingers, time);
    });
    m_input->processFilters([&fingers, &time](auto &&filter) {
        return filter->pinchGestureBegin(fingers, time);
    });
#endif
    m_ignoreEvents = false;
}

void KWinInputBackend::touchpadSwipeBlockingStopped(uint32_t fingers)
{
    m_ignoreEvents = true;
    const auto time = timestamp();
#ifdef KWIN_6_5_OR_GREATER
    KWin::PointerSwipeGestureBeginEvent event{
        .fingerCount = static_cast<int>(fingers),
        .time = time,
    };
    m_input->processSpies(&KWin::InputEventSpy::swipeGestureBegin, &event);
    m_input->processFilters(&KWin::InputEventFilter::swipeGestureBegin, &event);
#else
    m_input->processSpies([&fingers, &time](auto &&spy) {
        spy->swipeGestureBegin(fingers, time);
    });
    m_input->processFilters([&fingers, &time](auto &&filter) {
        return filter->swipeGestureBegin(fingers, time);
    });
#endif
    m_ignoreEvents = false;
}

void KWinInputBackend::kwinDeviceAdded(KWin::InputDevice *kwinDevice)
{
    if (kwinDevice == std::dynamic_pointer_cast<KWinInputEmitter>(g_inputEmitter)->device()) {
        return;
    }

    auto device = KWinInputDevice::tryCreate(this, kwinDevice);
    if (!device || deviceProperties(device.get()).ignore()) {
        return;
    }

    LibevdevComplementaryInputBackend::addDevice(device.get());
    InputBackend::addDevice(device.get());
    m_devices.push_back(std::move(device));
}

void KWinInputBackend::kwinDeviceRemoved(const KWin::InputDevice *kwinDevice)
{
    for (auto it = m_devices.begin(); it != m_devices.end(); it++) {
        const auto *device = it->get();
        if (device->kwinDevice() == kwinDevice) {
            removeDevice(device);
            m_devices.erase(it);
            return;
        }
    }
}

KWinInputDevice *KWinInputBackend::findDevice(KWin::InputDevice *kwinDevice)
{
    for (const auto &device : m_devices) {
        if (device->kwinDevice() == kwinDevice) {
            return device.get();
        }
    }
    return {};
}

void KWinInputBackend::KeyboardModifierSpy::keyboardKey(KWin::KeyboardKeyEvent *event)
{
    auto *backend = dynamic_cast<KWinInputBackend *>(g_inputBackend.get());
    if (backend->m_ignoreEvents) {
        return;
    }

    if (auto *device = backend->findDevice(event->device)) {
        device->setKeyState(event->nativeScanCode, event->state == KWin::KeyboardKeyState::Pressed);
    }
}

}