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

#include <libinputactions/input/InputEventHandler.h>

#include <QTimer>

namespace libinputactions
{

class Stroke;

/**
 * Collects input events and forwards them to event handlers.
 *
 * Primary backends are responsible for managing (adding and removing) devices. Complementary backends are only allowed to set properties when a device is
 * being added.
 *
 * Backends must ignore events when the session is locked or when m_ignoreEvents is set to true.
 *
 * To re-initialize the backend, call reset() and then initialize().
 * @see reset
 * @see initialize
 */
class InputBackend
{
    INPUTACTIONS_DECLARE_SINGLETON(InputBackend)

public:
    virtual ~InputBackend() = default;

    void addEventHandler(std::unique_ptr<InputEventHandler> handler);
    /**
     * This method should be used in order to prevent feedback loops when input is being emitted.
     * @param value Whether to ignore all input events.
     */
    void setIgnoreEvents(const bool &value);
    /**
     * Polls and handles events from all devices until there are no events left in the queue.
     */
    virtual void poll();

    std::vector<InputDevice *> devices() const;
    /**
     * @param properties Custom properties that will override the ones that were detected automatically.
     * @remark Custom properties will not be applied to devices that have already been added to the backend.
     */
    void addCustomDeviceProperties(const QString &name, const InputDeviceProperties &properties);

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
     * Removes all event handlers, devices and custom properties. Backend must be initialized in order to be used again.
     * @see initialize
     */
    void reset();

protected:
    InputBackend();

    void addDevice(std::unique_ptr<InputDevice> device);
    void removeDevice(InputDevice *device);
    /**
     * @returns The device with the specified name or nullptr if not found.
     */
    InputDevice *findDevice(const QString &name) const;

    /**
     * Backends should add device properties in this method.
     */
    virtual void deviceAdded(InputDevice *device);
    virtual void deviceRemoved(const InputDevice *device);

    /**
     * @param event Events with a nullptr sender will be ignored.
     * @return Whether the event should be blocked.
     */
    bool handleEvent(const InputEvent *event);

    void finishStrokeRecording();

    std::vector<std::unique_ptr<InputEventHandler>> m_handlers;
    bool m_ignoreEvents = false;

    bool m_isRecordingStroke = false;
    std::vector<QPointF> m_strokePoints;
    QTimer m_strokeRecordingTimeoutTimer;

private:
    std::function<void(const Stroke &stroke)> m_strokeCallback;

    std::vector<std::unique_ptr<InputDevice>> m_devices;
    std::map<QString, InputDeviceProperties> m_customDeviceProperties;
};

}