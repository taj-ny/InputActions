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

#include "window.h"

#include <utility>

namespace libinputactions
{

std::shared_ptr<Window> WindowProvider::active() const
{
    return {};
}

std::shared_ptr<Window> WindowProvider::underPointer() const
{
    return {};
}

WindowProvider *WindowProvider::instance()
{
    return s_instance.get();
}

void WindowProvider::setInstance(std::unique_ptr<WindowProvider> instance)
{
    s_instance = std::move(instance);
}

std::unique_ptr<WindowProvider> WindowProvider::s_instance = std::make_unique<WindowProvider>();

std::optional<QRectF> Window::geometry() const
{
    return {};
}

std::optional<QString> Window::title() const
{
    return {};
}

std::optional<QString> Window::resourceClass() const
{
    return {};
}

std::optional<QString> Window::resourceName() const
{
    return {};
}

std::optional<bool> Window::fullscreen() const
{
    return {};
}

std::optional<bool> Window::maximized() const
{
    return {};
}

}