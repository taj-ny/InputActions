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

#include "InputDevice.h"

namespace InputActions
{

class Condition;

/**
 * Applies a set of properties to all devices that satisfy the condition (if present).
 */
class InputDeviceRule
{
public:
    InputDeviceRule();
    ~InputDeviceRule();

    /**
     * May be nullptr.
     */
    const std::shared_ptr<Condition> &condition() const { return m_condition; }
    void setCondition(std::shared_ptr<Condition> value) { m_condition = std::move(value); }

    InputDeviceProperties &properties() { return m_properties; }
    const InputDeviceProperties &properties() const { return m_properties; }
    void setProperties(InputDeviceProperties value) { m_properties = std::move(value); }

private:
    std::shared_ptr<Condition> m_condition;
    InputDeviceProperties m_properties;
};

}