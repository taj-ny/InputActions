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

#include "enums.h"
#include <QString>
#include <libinputactions/actions/TriggerAction.h>
#include <libinputactions/config/ConfigIssueManager.h>
#include <libinputactions/config/Node.h>
#include <libinputactions/globals.h>
#include <libinputactions/interfaces/CursorShapeProvider.h>
#include <libinputactions/triggers/DirectionalMotionTrigger.h>
#include <unordered_map>

namespace InputActions
{

NODEPARSER_ENUM(ComparisonOperator, "operator",
                (std::unordered_map<QString, ComparisonOperator>{
                    {"==", ComparisonOperator::EqualTo},
                    {"!=", ComparisonOperator::NotEqualTo},
                    {">", ComparisonOperator::GreaterThan},
                    {">=", ComparisonOperator::GreaterThanOrEqual},
                    {"<", ComparisonOperator::LessThan},
                    {"<=", ComparisonOperator::LessThanOrEqual},
                    {"contains", ComparisonOperator::Contains},
                    {"between", ComparisonOperator::Between},
                    {"matches", ComparisonOperator::Regex},
                    {"one_of", ComparisonOperator::OneOf},
                }))
NODEPARSER_ENUM(CursorShape, "cursor shape", CURSOR_SHAPES)
NODEPARSER_ENUM(InputDeviceType, "input device type",
                (std::unordered_map<QString, InputDeviceType>{
                    {"keyboard", InputDeviceType::Keyboard},
                    {"mouse", InputDeviceType::Mouse},
                    {"touchpad", InputDeviceType::Touchpad},
                    {"touchscreen", InputDeviceType::Touchscreen},
                }))
NODEPARSER_ENUM(On, "action event",
                (std::unordered_map<QString, On>{
                    {"begin", On::Begin},
                    {"cancel", On::Cancel},
                    {"end", On::End},
                    {"end_cancel", On::EndCancel},
                    {"tick", On::Tick},
                    {"update", On::Update},
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

NODEPARSER_ENUM(Qt::KeyboardModifier, "keyboard modifier",
                (std::unordered_map<QString, Qt::KeyboardModifier>{
                    {"alt", Qt::KeyboardModifier::AltModifier},
                    {"ctrl", Qt::KeyboardModifier::ControlModifier},
                    {"meta", Qt::KeyboardModifier::MetaModifier},
                    {"shift", Qt::KeyboardModifier::ShiftModifier},
                }))

}