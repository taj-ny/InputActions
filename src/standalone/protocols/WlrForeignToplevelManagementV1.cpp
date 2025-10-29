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

#include "WlrForeignToplevelManagementV1.h"

WlrForeignToplevelManagementV1::WlrForeignToplevelManagementV1()
    : WaylandProtocol(zwlr_foreign_toplevel_manager_v1_interface.name)
{
}

WlrForeignToplevelManagementV1::~WlrForeignToplevelManagementV1()
{
    if (m_manager) {
        zwlr_foreign_toplevel_manager_v1_destroy(m_manager);
    }
}

std::shared_ptr<WlrForeignToplevelManagementV1Window> WlrForeignToplevelManagementV1::activeWindow()
{
    return m_activeWindow;
}

void WlrForeignToplevelManagementV1::bind(wl_registry *registry, uint32_t name, uint32_t version)
{
    WaylandProtocol::bind(registry, name, version);

    static const zwlr_foreign_toplevel_manager_v1_listener listener(&WlrForeignToplevelManagementV1::handleToplevel);

    m_manager = static_cast<zwlr_foreign_toplevel_manager_v1 *>(wl_registry_bind(registry, name, &zwlr_foreign_toplevel_manager_v1_interface, version));
    zwlr_foreign_toplevel_manager_v1_add_listener(m_manager, &listener, this);
}

void WlrForeignToplevelManagementV1::handleToplevel(void *data, zwlr_foreign_toplevel_manager_v1 *manager, zwlr_foreign_toplevel_handle_v1 *toplevel)
{
    static const zwlr_foreign_toplevel_handle_v1_listener listener{
        .title = &WlrForeignToplevelManagementV1::handleTitle,
        .app_id = &WlrForeignToplevelManagementV1::handleAppId,
        .output_enter = INPUTACTIONS_NOOP_3,
        .output_leave = INPUTACTIONS_NOOP_3,
        .state = &WlrForeignToplevelManagementV1::handleState,
        .done = INPUTACTIONS_NOOP_2,
        .closed = &WlrForeignToplevelManagementV1::handleClosed,
        .parent = INPUTACTIONS_NOOP_3,
    };

    auto window = std::make_shared<WlrForeignToplevelManagementV1Window>();
    zwlr_foreign_toplevel_handle_v1_add_listener(toplevel, &listener, window.get());
    static_cast<WlrForeignToplevelManagementV1 *>(data)->m_windows.emplace_back(window);
}

void WlrForeignToplevelManagementV1::handleTitle(void *data, struct zwlr_foreign_toplevel_handle_v1 *handle, const char *title)
{
    static_cast<WlrForeignToplevelManagementV1Window *>(data)->m_title = QString(title);
}

void WlrForeignToplevelManagementV1::handleAppId(void *data, zwlr_foreign_toplevel_handle_v1 *handle, const char *appId)
{
    static_cast<WlrForeignToplevelManagementV1Window *>(data)->m_resourceClass = QString(appId);
}

void WlrForeignToplevelManagementV1::handleState(void *data, zwlr_foreign_toplevel_handle_v1 *handle, wl_array *state)
{
    auto *self = g_wlrForeignToplevelManagementV1.get();
    auto *window = static_cast<WlrForeignToplevelManagementV1Window *>(data);
    window->m_fullscreen = false;
    window->m_maximized = false;
    for (auto i = 0; i < state->size; i++) {
        switch (static_cast<zwlr_foreign_toplevel_handle_v1_state *>(state->data)[i]) {
            case ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_MAXIMIZED:
                window->m_maximized = true;
                break;
            case ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_ACTIVATED: {
                auto it = std::ranges::find_if(self->m_windows, [window](auto &other) {
                    return window == other.get();
                });
                self->m_activeWindow = it == self->m_windows.end() ? nullptr : *it;
                break;
            }
            case ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_FULLSCREEN:
                window->m_fullscreen = true;
                break;
        }
    }
}

void WlrForeignToplevelManagementV1::handleClosed(void *data, zwlr_foreign_toplevel_handle_v1 *handle)
{
    auto *self = g_wlrForeignToplevelManagementV1.get();
    auto *window = static_cast<WlrForeignToplevelManagementV1Window *>(data);
    self->m_windows.erase(std::ranges::find_if(self->m_windows, [window](auto &other) {
        return window == other.get();
    }));
    if (self->m_activeWindow.get() == window) {
        self->m_activeWindow = {};
    }
}

std::shared_ptr<InputActions::Window> WlrForeignToplevelManagementV1WindowProvider::activeWindow()
{
    return g_wlrForeignToplevelManagementV1->activeWindow();
}

std::optional<QString> WlrForeignToplevelManagementV1Window::title()
{
    return m_title;
}

std::optional<QString> WlrForeignToplevelManagementV1Window::resourceClass()
{
    return m_resourceClass;
}

std::optional<bool> WlrForeignToplevelManagementV1Window::fullscreen()
{
    return m_fullscreen;
}

std::optional<bool> WlrForeignToplevelManagementV1Window::maximized()
{
    return m_maximized;
}