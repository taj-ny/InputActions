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

#include "DBusInterface.h"
#include <memory>

namespace libinputactions
{

class InputBackend;

class InputActions
{
public:
    /**
     * Runs the specified function on the main thread. If the current thread is the main thread, the function is executed immediately. Blocking calls introduce
     * action latency and should be used as little as possible.
     */
    void runOnMainThread(std::function<void()> &&function, bool block = true) const;

protected:
    InputActions(std::unique_ptr<InputBackend> inputBackend);
    virtual ~InputActions();

private:
    QThread *m_mainThread;
    DBusInterface m_dbusInterface;
};

inline InputActions *g_inputActions;

}