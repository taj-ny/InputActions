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

#include <QPointF>
#include <set>

namespace InputActions
{

/**
 * Virtual device for emitting anonymous mouse events.
 */
class VirtualMouse
{
public:
    virtual ~VirtualMouse() = default;

    /**
     * Must be called by the overriding method in order to track pressed buttons.
     */
    virtual void mouseButton(uint32_t button, bool state);
    virtual void mouseMotion(const QPointF &pos) {}
    virtual void mouseWheel(const QPointF &delta) {}

protected:
    /**
     * Puts the device in a neutral state. Call in the deriving class' destructor.
     */
    void reset();

private:
    std::set<uint32_t> m_pressedButtons;
};

}