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

#include "NodeParser.h"
#include <libinputactions/config/Config.h>
#include <libinputactions/config/Node.h>
#include <unordered_map>
#include <QString>
#include <libinputactions/triggers/DirectionalMotionTrigger.h>
#include <libinputactions/globals.h>

namespace InputActions
{

#define NODEPARSER_FLAGS(T, name, map)                                                          \
    template<> \
    void NodeParser<T>::parse(const Node *node, T &result)                                                  \
    {                                                                                           \
        result = {0};                                                                      \
        for (const auto &raw : node->as<QStringList>()) {                                      \
            if (!map.contains(raw)) {                                                           \
                throw ConfigParserException(node, QString("Invalid %1 ('%2').").arg(name, raw)); \
            }                                                                                   \
            result |= map.at(raw);                                                              \
        }                                                                                       \
    }

NODEPARSER_FLAGS(Qt::KeyboardModifiers, "keyboard modifier",
              (std::unordered_map<QString, Qt::KeyboardModifier>{
                  {"alt", Qt::KeyboardModifier::AltModifier},
                  {"ctrl", Qt::KeyboardModifier::ControlModifier},
                  {"meta", Qt::KeyboardModifier::MetaModifier},
                  {"shift", Qt::KeyboardModifier::ShiftModifier},
              }))
NODEPARSER_FLAGS(InputDeviceTypes, "input device type",
              (std::unordered_map<QString, InputDeviceType>{
                  {"keyboard", InputDeviceType::Keyboard},
                  {"mouse", InputDeviceType::Mouse},
                  {"touchpad", InputDeviceType::Touchpad},
                  {"touchscreen", InputDeviceType::Touchscreen},
              }))

}