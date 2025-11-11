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

#include <QTimer>

namespace InputActions
{

class InputDevice;
class InputDeviceProperties;
class InputDeviceRule;
class InputEvent;
class InputEventHandler;
class KeyboardTriggerHandler;
class MouseTriggerHandler;
class PointerTriggerHandler;
class Stroke;
class TouchpadTriggerHandler;

/**
 * Collects input events and forwards them to event handlers. Handlers can only be set before initialization.
 *
 * Primary backends are responsible for managing (adding and removing) devices. Complementary backends are only allowed to set properties when a device is
 * being added.
 *
 * On keyboard key events, the backend must call InputDevice::updateModifiers before InputBackend::handleEvent.
 *
 * Backends must ignore events when m_ignoreEvents is set to true.
 *
 * To re-initialize the backend, call reset() and then initialize().
 * @see reset
 * @see initialize
 */
class InputBackend
{
public:
    InputBackend();
    virtual ~InputBackend();

    /**
     * This method should be used in order to prevent feedback loops when input is being emitted.
     * @param value Whether to ignore all input events.
     */
    void setIgnoreEvents(bool value);
    /**
     * Polls and handles events from all devices until there are no events left in the queue.
     */
    virtual void poll();

    /**
     * Detects and adds devices.
     */
    virtual void initialize();

    /**
     * @param callback Will be called when the stroke has been recorded.
     * @remark Calling this when a stroke is already being recorded will result in the previous callback never being called.
     */
    void recordStroke(const std::function<void(const Stroke &stroke)> &callback);

    /**
     * Evaluates device rules for the specified device and returns the properties without modifying the device's properties. Use this for devices that have not
     * been added to the backend yet, otherwise use InputDevice::properties().
     */
    InputDeviceProperties deviceProperties(const InputDevice *device) const;

    std::vector<InputDevice *> devices();

    /**
     * @return Currently pressed keyboard modifiers, accumulated from all devices.
     * @remark Key events that have been ignored by the input backend will not be used to update the modifier state. For example, clearing modifiers will not
     * update the modifier state to none. This allows gestures with keyboard modifier conditions to be used again.
     */
    Qt::KeyboardModifiers keyboardModifiers() const;

    /**
     * Removes all event handlers, devices and custom properties. Backend must be initialized in order to be used again.
     * @see initialize
     */
    virtual void reset();

    /**
     * Rules are evaluated in reverse order when a device is added.
     */
    std::vector<InputDeviceRule> m_deviceRules;

    std::unique_ptr<KeyboardTriggerHandler> m_keyboardTriggerHandler;
    std::unique_ptr<MouseTriggerHandler> m_mouseTriggerHandler;
    std::unique_ptr<PointerTriggerHandler> m_pointerTriggerHandler;

    std::function<std::unique_ptr<TouchpadTriggerHandler>(InputDevice *device)> m_touchpadTriggerHandlerFactory;

protected:
    /**
     * Backends should add device properties in this method.
     */
    virtual void deviceAdded(InputDevice *device);
    virtual void deviceRemoved(const InputDevice *device);
    void createEventHandlerChain();

    /**
     * Events with a nullptr sender will be ignored.
     * @returns Whether the event should be blocked.
     */
    bool handleEvent(const InputEvent &event);

    void finishStrokeRecording();

    bool m_ignoreEvents = false;

    bool m_isRecordingStroke = false;
    std::vector<QPointF> m_strokePoints;
    QTimer m_strokeRecordingTimeoutTimer;

private:
    void applyDeviceProperties(const InputDevice *device, InputDeviceProperties &properties) const;

    std::function<void(const Stroke &stroke)> m_strokeCallback;

    std::vector<InputEventHandler *> m_eventHandlerChain;

    std::vector<InputDevice *> m_devices;
};

inline std::unique_ptr<InputBackend> g_inputBackend;

}