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
#include <libinputactions/actions/TriggerAction.h>
#include <libinputactions/interfaces/CursorShapeProvider.h>
#include <libinputactions/triggers/DirectionalMotionTrigger.h>
#include <libinputactions/globals.h>

namespace InputActions
{

#define NODEPARSER_ENUM(T, name, map)                                                       \
    template<>                                                                              \
    void NodeParser<T>::parse(const Node *node, T &result)                                  \
    {                                                                                       \
        const auto raw = node->as<QString>();                                               \
        if (!map.contains(raw)) {                                                           \
             throw ConfigParserException(node, QString("Invalid %1 '%2'.").arg(name, raw)); \
        }                                                                                   \
        result = map.at(raw);                                                               \
    }

NODEPARSER_ENUM(On, "action event",
             (std::unordered_map<QString, On>{
                 {"begin", On::Begin},
                 {"cancel", On::Cancel},
                 {"end", On::End},
                 {"end_cancel", On::EndCancel},
                 {"tick", On::Tick},
                 {"update", On::Update},
             }))
NODEPARSER_ENUM(CursorShape, "cursor shape", CURSOR_SHAPES)
NODEPARSER_ENUM(Qt::MouseButton, "mouse button",
             (std::unordered_map<QString, Qt::MouseButton>{
                 {"left", Qt::MouseButton::LeftButton},
                 {"middle", Qt::MouseButton::MiddleButton},
                 {"right", Qt::MouseButton::RightButton},
                 {"back", Qt::MouseButton::ExtraButton1},
                 {"forward", Qt::MouseButton::ExtraButton2},
                 {"extra1", Qt::MouseButton::ExtraButton1},
                 {"extra2", Qt::MouseButton::ExtraButton2},
                 {"extra3", Qt::MouseButton::ExtraButton3},
                 {"extra4", Qt::MouseButton::ExtraButton4},
                 {"extra5", Qt::MouseButton::ExtraButton5},
                 {"extra6", Qt::MouseButton::ExtraButton6},
                 {"extra7", Qt::MouseButton::ExtraButton7},
                 {"extra8", Qt::MouseButton::ExtraButton8},
                 {"extra9", Qt::MouseButton::ExtraButton9},
                 {"extra10", Qt::MouseButton::ExtraButton10},
                 {"extra11", Qt::MouseButton::ExtraButton11},
                 {"extra12", Qt::MouseButton::ExtraButton12},
                 {"extra13", Qt::MouseButton::ExtraButton13},
             }))
NODEPARSER_ENUM(PinchDirection, "pinch direction",
             (std::unordered_map<QString, PinchDirection>{
                 {"in", PinchDirection::In},
                 {"out", PinchDirection::Out},
                 {"any", PinchDirection::Any},
             }))
NODEPARSER_ENUM(RotateDirection, "rotate direction",
             (std::unordered_map<QString, RotateDirection>{
                 {"clockwise", RotateDirection::Clockwise},
                 {"counterclockwise", RotateDirection::Counterclockwise},
                 {"any", RotateDirection::Any},
             }))
NODEPARSER_ENUM(SwipeDirection, "swipe direction",
             (std::unordered_map<QString, SwipeDirection>{
                 {"left", SwipeDirection::Left},
                 {"right", SwipeDirection::Right},
                 {"up", SwipeDirection::Up},
                 {"down", SwipeDirection::Down},
                 {"up_down", SwipeDirection::UpDown},
                 {"left_right", SwipeDirection::LeftRight},
                 {"any", SwipeDirection::Any},
             }))
NODEPARSER_ENUM(TriggerSpeed, "trigger speed",
             (std::unordered_map<QString, TriggerSpeed>{
                 {"fast", TriggerSpeed::Fast},
                 {"slow", TriggerSpeed::Slow},
                 {"any", TriggerSpeed::Any},
             }))

}