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

#include <memory>

namespace InputActions
{

class Condition;
class InputDevice;
class Node;
class TouchpadTriggerHandler;
class TouchscreenTriggerHandler;
class VariableManager;

std::shared_ptr<Condition> parseCondition(const Node *node, const VariableManager *variableManager = nullptr);

std::unique_ptr<TouchpadTriggerHandler> parseTouchpadTriggerHandler(const Node *node, InputDevice *device);
std::unique_ptr<TouchscreenTriggerHandler> parseTouchscreenTriggerHandler(const Node *node, InputDevice *device);

}
