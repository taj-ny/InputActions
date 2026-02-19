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

#include <libinputactions/config/Node.h>
#include <memory>
#include <optional>

namespace InputActions
{

template<typename T>
void loadMember(T &member, const Node *node)
{
    if (node) {
        member = node->as<T>();
    }
}
template<typename T>
void loadMember(std::optional<T> &member, const Node *node)
{
    if (node) {
        member = node->as<T>();
    }
}

template<typename T, typename TObject>
void loadSetter(TObject *object, void (TObject::*setter)(T), const Node *node)
{
    if (node) {
        (object->*setter)(node->as<T>());
    }
}

template<typename T, typename TObject>
void loadSetter(const std::unique_ptr<TObject> &object, void (TObject::*setter)(T), const Node *node)
{
    if (node) {
        (object.get()->*setter)(node->as<T>());
    }
}

template<typename T, typename TObject>
void loadSetter(const std::shared_ptr<TObject> &object, void (TObject::*setter)(T), const Node *node)
{
    if (node) {
        (object.get()->*setter)(node->as<T>());
    }
}

template<typename T, typename TObject>
void loadSetter(TObject &object, void (TObject::*setter)(T), const Node *node)
{
    if (node) {
        (object.*setter)(node->as<T>());
    }
}

}