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

#include <QString>
#include <libinputactions/handlers/TriggerHandler.h>
#include <memory>
#include <set>

namespace libinputactions
{

/**
 * Handles device input events. Can handle multiple devices of different types.
 */
class InputEventHandler
{
public:
    /**
     * @return Whether the event should be blocked. False if trigger handler had not been set or the device name
     * doesn't match.
     */
    bool handleEvent(const InputEvent *event);

    /**
     * Mutually exclusive with setDeviceNameWhitelist.
     * @param blacklist Devices to be ignored.
     */
    void setDeviceNameBlacklist(const std::set<QString> &blacklist);
    /**
     * Mutually exclusive with setDeviceNameBlacklist.
     * @param whitelist Devices to not be ignored.
     */
    void setDeviceNameWhitelist(const std::set<QString> &whitelist);

    void setTriggerHandler(std::unique_ptr<TriggerHandler> handler);

private:
    bool matchesDevice(const QString &name) const;

    std::set<QString> m_deviceNameBlacklist;
    std::set<QString> m_deviceNameWhitelist;

    std::unique_ptr<TriggerHandler> m_triggerHandler;
};

}