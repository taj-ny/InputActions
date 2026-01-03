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

#include <libinput-cpp/LibinputPathContext.h>
#include <libinput.h>
#include <libinputactions/input/backends/LibevdevComplementaryInputBackend.h>
#include <libinputactions/input/backends/LibinputInputBackend.h>
#include <poll.h>

namespace InputActions
{

class LibinputDevice;
class LibevdevUinputDevice;

struct LibinputEventsProcessingResult
{
    bool block{};
    uint32_t eventCount{};
};

class StandaloneInputBackend : public LibinputInputBackend
{
    Q_OBJECT

public:
    StandaloneInputBackend();
    ~StandaloneInputBackend() override;

    void initialize() override;
    void reset() final;

    LibevdevUinputDevice *outputDevice(const InputDevice *device) const;

private slots:
    void inotifyTimerTick();
    void deviceInitializationRetryTimerTick();

private:
    struct ExtraDeviceData;

    void poll();

    void evdevDeviceAdded(const QString &path);
    /**
     * ExtraDeviceData::libinputDevice may be nullptr and has to be initialized later when the newly created device can be opened.
     * @return True if the device was added or rejected not as a result of an error, false if an error occurred.
     */
    bool tryAddEvdevDevice(const QString &path);
    void finishLibinputDeviceInitialization(InputDevice *device, ExtraDeviceData *data);
    void evdevDeviceRemoved(const QString &path);

    bool handleEvent(InputDevice *sender, const LibinputEvent *event);
    LibinputEventsProcessingResult handleLibinputEvents(InputDevice *device, LibinputPathContext &libinput);

    /**
     * @return Whether the specified device is in a neutral state.
     */
    bool isDeviceNeutral(const InputDevice *device, const ExtraDeviceData *data);
    /**
     * Resets the output device of the specified grabbed device into a neutral state.
     */
    void resetDevice(const InputDevice *device, const ExtraDeviceData *data);
    /**
     * Copies the current state of the specified grabbed touchpad to its neutral output device.
     */
    void copyTouchpadState(const ExtraDeviceData *data) const;

    int m_inotifyFd;
    std::unique_ptr<QSocketNotifier> m_inotifyNotifier;

    /**
     * Contains devices that have failed to initialize due to the first open failing.
     * <path, attempts>
     */
    std::map<QString, uint32_t> m_deviceInitializationQueue;
    QTimer m_deviceInitializationRetryTimer;

    struct ExtraDeviceData
    {
        ExtraDeviceData();
        ~ExtraDeviceData();

        Q_DISABLE_COPY_MOVE(ExtraDeviceData);

        std::shared_ptr<LibevdevDevice> libevdev;
        QString path;
        uint32_t initializationAttempts{};

        /**
         * Libinput context containing only libinputDevice.
         */
        LibinputPathContext libinput;
        /**
         * If the device is grabbed, this is the same device as libinputInjectionDevice, otherwise it is the real device.
         */
        LibinputDevice *libinputDevice{};

        /**
         * The virtual device for injecting raw evdev events into libinput, as there is no API for that. Grabbed by libinput.
         *
         * Only available if the device is grabbed.
         */
        std::unique_ptr<LibevdevUinputDevice> libinputEventInjectionDevice{};

        /**
         * The virtual device where non-filtered events are written to.
         *
         * Only available if the device is grabbed.
         */
        std::unique_ptr<LibevdevUinputDevice> outputDevice{};

        bool touchpadBlocked{};
        bool touchpadNeutral = true;
        QTimer touchpadStateResetTimer;
    };
    std::vector<std::pair<std::unique_ptr<InputDevice>, std::unique_ptr<ExtraDeviceData>>> m_devices;
};

}