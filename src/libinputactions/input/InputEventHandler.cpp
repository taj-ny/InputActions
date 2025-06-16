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

#include "InputEventHandler.h"

namespace libinputactions
{

bool InputEventHandler::handleEvent(const InputEvent *event)
{
    if (!m_triggerHandler || !matchesDevice(event->sender()->name())) {
        return false;
    }
    return m_triggerHandler->handleEvent(event);
}

bool InputEventHandler::matchesDevice(const QString &name) const
{
    if (!m_deviceNameWhitelist.empty()) {
        return m_deviceNameWhitelist.contains(name);
    } else if (!m_deviceNameBlacklist.empty()) {
        return !m_deviceNameBlacklist.contains(name);
    }
    return true;
}

void InputEventHandler::setDeviceNameBlacklist(const std::set<QString> &blacklist)
{
    m_deviceNameBlacklist = blacklist;
}

void InputEventHandler::setDeviceNameWhitelist(const std::set<QString> &whitelist)
{
    m_deviceNameWhitelist = whitelist;
}

void InputEventHandler::setTriggerHandler(std::unique_ptr<TriggerHandler> handler)
{
    m_triggerHandler = std::move(handler);
}

}