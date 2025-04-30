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

#include <memory>

#include <QPointF>

namespace libinputactions
{

class Pointer
{
public:
    virtual ~Pointer() = default;

    /**
     * @return Global position in pixels or std::nullopt if not available.
     */
    virtual std::optional<QPointF> globalPosition() const;
    /**
     * @return Position on the current screen ranging from (0,0) to (1,1), std::nullopt if not available.
     */
    virtual std::optional<QPointF> screenPosition() const;
    virtual void setPosition(const QPointF &position);

    static Pointer *instance();
    static void setInstance(std::unique_ptr<Pointer> instance);

protected:
    Pointer() = default;

private:
    static std::unique_ptr<Pointer> s_instance;
};

}