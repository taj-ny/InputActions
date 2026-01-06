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
#include <linux/input-event-codes.h>
#include <unordered_set>

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
class TouchpadTriggerHandler;
class TouchscreenTriggerHandler;

/**
 * Collects input events and forwards them to event handlers. Handlers can only be set before initialization.
 *
 * Primary backends are responsible for managing (adding and removing) devices. When a device is added, the primary backend must call InputBackend::addDevice
 * and optionally, complementary backends' addDevice methods before that if available. When a device is removed, the primary backend must call deviceRemoved.
 *
 * Complementary backends are only allowed to set properties in their addDevice methods.
 *
 * On keyboard key events, the backend must call InputDevice::setKeyState before InputBackend::handleEvent.
 *
 * Backends must ignore events when m_ignoreEvents is set to true.
 *
 * To re-initialize the backend, call reset() and then initialize().
 * @see reset
 * @see initialize
 */
class InputBackend : public QObject
{
    Q_OBJECT

public:
    InputBackend();
    virtual ~InputBackend();

    /**
     * This method should be used in order to prevent feedback loops when input is being emitted.
     * @param value Whether to ignore all input events.
     */
    void setIgnoreEvents(bool value);

    /**
     * Detects and adds devices.
     */
    virtual void initialize() {};

    /**
     * Evaluates device rules for the specified device and returns the properties without modifying the device's properties. Use this for devices that have not
     * been added to the backend yet, otherwise use InputDevice::properties().
     */
    InputDeviceProperties deviceProperties(const InputDevice *device) const;

    std::vector<InputDevice *> devices();
    /**
     * Use in case the device is not provided by the compositor for some reason.
     * @return May be nullptr.
     */
    InputDevice *firstTouchpad() const;
    /**
     * Use in case the device is not provided by the compositor for some reason.
     * @return May be nullptr.
     */
    InputDevice *firstTouchscreen() const;
    /**
     * @return The touchscreen currently in use or nullptr if not available.
     */
    InputDevice *currentTouchscreen() const { return m_currentTouchscreen; }

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
    void setDeviceRules(std::vector<InputDeviceRule> rules);

    void setKeyboardTriggerHandler(std::unique_ptr<KeyboardTriggerHandler> value);
    void setMouseTriggerHandler(std::unique_ptr<MouseTriggerHandler> value);
    void setPointerTriggerHandler(std::unique_ptr<PointerTriggerHandler> value);
    void setTouchpadTriggerHandlerFactory(std::function<std::unique_ptr<TouchpadTriggerHandler>(InputDevice *device)> value);
    void setTouchscreenTriggerHandlerFactory(std::function<std::unique_ptr<TouchscreenTriggerHandler>(InputDevice *device)> value);

    /**
     * A combination of keyboard keys, that when held for a specific amount of time, will cause InputActions to enter a suspended state.
     */
    void setEmergencyCombination(std::unordered_set<uint32_t> value) { m_emergencyCombination = value; }

protected:
    void addDevice(InputDevice *device);
    virtual void removeDevice(const InputDevice *device);
    void createEventHandlerChain();

    /**
     * Events with a nullptr sender will be ignored.
     * @returns Whether the event should be blocked.
     */
    bool handleEvent(const InputEvent &event);

    bool m_ignoreEvents = false;

private slots:
    void onEmergencyCombinationTimerTimeout();

private:
    void applyDeviceProperties(const InputDevice *device, InputDeviceProperties &properties) const;

    std::vector<InputEventHandler *> m_eventHandlerChain;
    std::vector<InputDevice *> m_devices;
    InputDevice *m_currentTouchscreen{};

    QTimer m_emergencyCombinationTimer;

    std::vector<InputDeviceRule> m_deviceRules;
    std::unique_ptr<KeyboardTriggerHandler> m_keyboardTriggerHandler;
    std::unique_ptr<MouseTriggerHandler> m_mouseTriggerHandler;
    std::unique_ptr<PointerTriggerHandler> m_pointerTriggerHandler;
    std::function<std::unique_ptr<TouchpadTriggerHandler>(InputDevice *device)> m_touchpadTriggerHandlerFactory;
    std::function<std::unique_ptr<TouchscreenTriggerHandler>(InputDevice *device)> m_touchscreenTriggerHandlerFactory;

    std::unordered_set<uint32_t> m_emergencyCombination = {KEY_BACKSPACE, KEY_SPACE, KEY_ENTER};
};

inline std::unique_ptr<InputBackend> g_inputBackend;

}