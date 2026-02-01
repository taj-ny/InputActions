/*
    Input Actions - Input handler that executes user-defined actions
    Copyright (C) 2024-2026 Marcin Woźniak

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

#include "EvdevVirtualKeyboard.h"
#include "EvdevVirtualMouse.h"
#include <libinput-cpp/PathContext.h>
#include <libinput.h>
#include <libinputactions/input/backends/LibevdevComplementaryInputBackend.h>
#include <libinputactions/input/backends/LibinputInputBackend.h>
#include <poll.h>

namespace InputActions
{

class StandaloneInputDevice;

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

    VirtualKeyboard *virtualKeyboard() override;
    VirtualMouse *virtualMouse() override;

    void initialize() override;
    void reset() final;

private slots:
    void inotifyTimerTick();
    void deviceInitializationRetryTimerTick();

private:
    void poll();

    void evdevDeviceAdded(const QString &path);
    /**
     * ExtraDeviceData::libinputDevice may be nullptr and has to be initialized later when the newly created device can be opened.
     * @return True if the device was added or rejected not as a result of an error, false if an error occurred.
     */
    bool tryAddEvdevDevice(const QString &path);
    void evdevDeviceRemoved(const QString &path);

    bool handleEvent(StandaloneInputDevice *sender, const libinput::Event &event);
    LibinputEventsProcessingResult handleLibinputEvents(StandaloneInputDevice *device, libinput::PathContext *libinputContext);

    int m_inotifyFd;
    std::unique_ptr<QSocketNotifier> m_inotifyNotifier;

    /**
     * Contains devices that have failed to initialize due to the first open failing.
     * <path, attempts>
     */
    std::map<QString, uint32_t> m_deviceInitializationQueue;
    QTimer m_deviceInitializationRetryTimer;

    std::vector<std::unique_ptr<StandaloneInputDevice>> m_devices;
    std::optional<EvdevVirtualKeyboard> m_virtualKeyboard;
    std::optional<EvdevVirtualMouse> m_virtualMouse;
};

}