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

#include <cassert>
#include <hyprland/src/plugins/HookSystem.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>
#include <string>
#undef HANDLE

namespace InputActions
{

template<auto T>
class HyprlandFunctionHook
{
public:
    HyprlandFunctionHook(void *handle, const std::string &sourceName, size_t sourceIndex = 0)
        : m_handle(handle)
    {
        const auto functions = HyprlandAPI::findFunctionsByName(handle, sourceName);
        assert(functions.size() - 1 >= sourceIndex);
        m_hook = HyprlandAPI::createFunctionHook(handle, functions[sourceIndex].address, (void *)T);
        m_hook->hook();
    }

    ~HyprlandFunctionHook() { HyprlandAPI::removeFunctionHook(m_handle, m_hook); }

    template<typename... Args>
    void operator()(Args &&...args) const
    {
        if (m_hook) {
            ((decltype(T))m_hook->m_original)(std::forward<Args>(args)...);
        }
    }

private:
    CFunctionHook *m_hook;
    void *m_handle;
};

}