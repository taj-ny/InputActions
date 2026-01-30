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

#include "WaylandProtocol.h"
#include "wlr-foreign-toplevel-management-unstable-v1.h"

namespace InputActions
{

class Client;

struct WlrForeignToplevelManagementV1Window
{
    QString title;
    QString resourceClass;
    bool fullscreen{};
    bool maximized{};
};

class WlrForeignToplevelManagementV1 : public WaylandProtocol
{
public:
    WlrForeignToplevelManagementV1(Client *client);
    ~WlrForeignToplevelManagementV1() override;

protected:
    void bind(wl_registry *registry, uint32_t name, uint32_t version) override;

private:
    static void handleToplevel(void *data, zwlr_foreign_toplevel_manager_v1 *manager, zwlr_foreign_toplevel_handle_v1 *toplevel);

    static void handleTitle(void *data, zwlr_foreign_toplevel_handle_v1 *handle, const char *title);
    static void handleAppId(void *data, zwlr_foreign_toplevel_handle_v1 *handle, const char *appId);
    static void handleState(void *data, zwlr_foreign_toplevel_handle_v1 *handle, wl_array *state);
    static void handleClosed(void *data, zwlr_foreign_toplevel_handle_v1 *handle);
    static void handleDone(void *data, zwlr_foreign_toplevel_handle_v1 *handle);

    zwlr_foreign_toplevel_manager_v1 *m_manager{};
    std::vector<std::unique_ptr<WlrForeignToplevelManagementV1Window>> m_windows;
    WlrForeignToplevelManagementV1Window *m_activeWindow;

    Client *m_client;

    inline static WlrForeignToplevelManagementV1 *self;
};

}