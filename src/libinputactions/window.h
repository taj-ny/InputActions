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

#include <QRectF>
#include <QString>

namespace libinputactions
{

class Window
{
public:
    virtual ~Window() = default;

    virtual std::optional<QString> id() const;
    virtual std::optional<QRectF> geometry() const;
    virtual std::optional<QString> title() const;
    virtual std::optional<QString> resourceClass() const;
    virtual std::optional<QString> resourceName() const;
    virtual std::optional<bool> maximized() const;
    virtual std::optional<bool> fullscreen() const;

protected:
    Window() = default;
};

class WindowProvider
{
public:
    WindowProvider() = default;
    virtual ~WindowProvider() = default;

    /**
     * @return The currently active window, or nullptr if not available.
     */
    virtual std::shared_ptr<Window> active() const;
    /**
     * @return The window under the pointer, or nullptr if not available.
     */
    virtual std::shared_ptr<Window> underPointer() const;

    static WindowProvider *instance();
    static void setInstance(std::unique_ptr<WindowProvider> instance);

private:
    static std::unique_ptr<WindowProvider> s_instance;
};

}