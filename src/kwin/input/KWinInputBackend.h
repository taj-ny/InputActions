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

#include "input.h"
#include "input_event_spy.h"
#include <libinputactions/input/backends/LibinputInputBackend.h>

struct KWinInputDevice
{
    KWin::InputDevice *kwinDevice;
    std::unique_ptr<InputActions::InputDevice> libinputactionsDevice;
};

/**
 * Installed before GlobalShortcutFilter, which is responsible for handling touchpad gestures.
 * @returns All methods that process events should return @c true to stop further event processing, @c false to pass to
 * next filter.
 */
class KWinInputBackend
    : public QObject
    , public InputActions::LibinputInputBackend
    , public KWin::InputEventFilter
{
public:
    KWinInputBackend();
    ~KWinInputBackend() override;

    void initialize() override;
    void reset() final;

#ifdef KWIN_6_5_OR_GREATER
    bool holdGestureBegin(KWin::PointerHoldGestureBeginEvent *event) override;
    bool holdGestureEnd(KWin::PointerHoldGestureEndEvent *event) override;
    bool holdGestureCancelled(KWin::PointerHoldGestureCancelEvent *event) override;

    bool swipeGestureBegin(KWin::PointerSwipeGestureBeginEvent *event) override;
    bool swipeGestureUpdate(KWin::PointerSwipeGestureUpdateEvent *event) override;
    bool swipeGestureEnd(KWin::PointerSwipeGestureEndEvent *event) override;
    bool swipeGestureCancelled(KWin::PointerSwipeGestureCancelEvent *event) override;

    bool pinchGestureBegin(KWin::PointerPinchGestureBeginEvent *event) override;
    bool pinchGestureUpdate(KWin::PointerPinchGestureUpdateEvent *event) override;
    bool pinchGestureEnd(KWin::PointerPinchGestureEndEvent *event) override;
    bool pinchGestureCancelled(KWin::PointerPinchGestureCancelEvent *event) override;
#else
    bool holdGestureBegin(int fingerCount, std::chrono::microseconds time) override;
    bool holdGestureEnd(std::chrono::microseconds time) override;
    bool holdGestureCancelled(std::chrono::microseconds time) override;

    bool swipeGestureBegin(int fingerCount, std::chrono::microseconds time) override;
    bool swipeGestureUpdate(const QPointF &delta, std::chrono::microseconds time) override;
    bool swipeGestureEnd(std::chrono::microseconds time) override;
    bool swipeGestureCancelled(std::chrono::microseconds time) override;

    bool pinchGestureBegin(int fingerCount, std::chrono::microseconds time) override;
    bool pinchGestureUpdate(qreal scale, qreal angleDelta, const QPointF &delta, std::chrono::microseconds time) override;
    bool pinchGestureEnd(std::chrono::microseconds time) override;
    bool pinchGestureCancelled(std::chrono::microseconds time) override;
#endif

    bool pointerAxis(KWin::PointerAxisEvent *event) override;
    bool pointerButton(KWin::PointerButtonEvent *event) override;
    bool pointerMotion(KWin::PointerMotionEvent *event) override;

    bool keyboardKey(KWin::KeyboardKeyEvent *event) override;

    void touchpadPinchBlockingStopped(uint32_t fingers) override;
    void touchpadSwipeBlockingStopped(uint32_t fingers) override;

private:
    void kwinDeviceAdded(KWin::InputDevice *kwinDevice);
    void kwinDeviceRemoved(const KWin::InputDevice *kwinDevice);
    InputActions::InputDevice *findInputActionsDevice(const KWin::InputDevice *kwinDevice);
    /**
     * @return The device that generated the last event.
     */
    InputActions::InputDevice *currentTouchpad();

    bool isMouse(const KWin::InputDevice *device) const;

    KWin::InputRedirection *m_input;
    std::vector<KWinInputDevice> m_devices;

    class KeyboardModifierSpy : public KWin::InputEventSpy
    {
        void keyboardKey(KWin::KeyboardKeyEvent *event) override;
    } m_keyboardModifierSpy;
};