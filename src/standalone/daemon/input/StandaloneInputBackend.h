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

#include <QSocketNotifier>
#include <libevdev/libevdev-uinput.h>
#include <libinput.h>
#include <libinputactions/input/backends/LibevdevComplementaryInputBackend.h>
#include <libinputactions/input/backends/LibinputInputBackend.h>
#include <libudev.h>
#include <poll.h>

namespace InputActions
{

struct LibinputEventsProcessingResult
{
    bool block{};
    uint32_t eventCount{};
};

class StandaloneInputBackend
    : public QObject
    , public LibinputInputBackend
{
    Q_OBJECT

public:
    StandaloneInputBackend();
    ~StandaloneInputBackend() override;

    void initialize() override;
    void reset() final;

    void poll() override;

    libevdev_uinput *outputDevice(const InputDevice *device) const;

private slots:
    void inotifyTimerTick();
    void deviceInitializationRetryTimerTick();

private:
    struct ExtraDeviceData;

    void evdevDeviceAdded(const QString &path);
    /**
     * ExtraDeviceData::libinputDevice may be nullptr and has to be initialized later when the newly created device can be opened.
     * @return True if the device was added or rejected not as a result of an error, false if an error occurred.
     */
    bool tryAddEvdevDevice(const QString &path);
    void finishLibinputDeviceInitialization(InputDevice *device, ExtraDeviceData *data);
    void evdevDeviceRemoved(const QString &path);

    bool handleEvent(InputDevice *sender, libinput_event *event);
    LibinputEventsProcessingResult handleLibinputEvents(InputDevice *device, libinput *libinput);

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

    libinput_interface m_libinputBlockingInterface;
    libinput_interface m_libinputNonBlockingInterface;

    int m_inotifyFd;
    std::unique_ptr<QSocketNotifier> m_inotifyNotifier;

    /**
     * Contains devices that have failed to initialize due to the first open failing.
     * <path, attempts>
     */
    std::map<QString, uint32_t> m_deviceInitializationQueue;
    QTimer m_deviceInitializationRetryTimer;

    static int openRestricted(const char *path, int flags, void *data);
    static int openRestrictedGrab(const char *path, int flags, void *data);
    static void closeRestricted(int fd, void *data);

    struct ExtraDeviceData
    {
        ExtraDeviceData() = default;
        ~ExtraDeviceData();

        ExtraDeviceData(const ExtraDeviceData &) = delete;
        ExtraDeviceData(ExtraDeviceData &&) = delete;
        ExtraDeviceData &operator=(const ExtraDeviceData &) = delete;
        ExtraDeviceData &operator=(ExtraDeviceData &&) = delete;

        uint32_t initializationAttempts{};

        struct libevdev *libevdev{};
        std::unique_ptr<QSocketNotifier> libevdevNotifier;
        /**
         * Absolute path of the device in /dev/input.
         */
        std::string path;

        /**
         * Libinput context containing only libinputDevice.
         */
        struct libinput *libinput{};
        std::unique_ptr<QSocketNotifier> libinputNotifier;
        /**
         * If the device is grabbed, this is the same device as libinputInjectionDevice, otherwise it is the real device.
         */
        libinput_device *libinputDevice{};

        /**
         * The virtual device for injecting raw evdev events into libinput, as there is no API for that. Grabbed by libinput.
         *
         * Only available if the device is grabbed.
         */
        libevdev_uinput *libinputEventInjectionDevice{};
        /**
         * Absolute path of libinputEventInjectionDevice.
         */
        std::string libinputEventInjectionDevicePath;

        /**
         * The virtual device where non-filtered events are written to.
         *
         * Only available if the device is grabbed.
         */
        libevdev_uinput *outputDevice{};
        /**
         * Absolute path of outputDevice.
         */
        std::string outputDevicePath;

        bool touchpadBlocked{};
        bool touchpadNeutral = true;
        QTimer touchpadStateResetTimer;
    };
    std::vector<std::pair<std::unique_ptr<InputDevice>, std::unique_ptr<ExtraDeviceData>>> m_devices;
};

}