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

#include "VariableManager.h"
#include "Variable.h"
#include <QLoggingCategory>
#include <QRegularExpression>
#include <libinputactions/input/Keyboard.h>
#include <libinputactions/interfaces/CursorShapeProvider.h>
#include <libinputactions/interfaces/PointerPositionGetter.h>
#include <libinputactions/interfaces/Window.h>
#include <libinputactions/interfaces/WindowProvider.h>

Q_LOGGING_CATEGORY(INPUTACTIONS_VARIABLE_MANAGER, "inputactions.variable.manager", QtWarningMsg)

namespace libinputactions
{

VariableManager::VariableManager()
{
    registerRemoteVariable<CursorShape>("cursor_shape", [](auto &value) {
        value = g_cursorShapeProvider->cursorShape();
    });
    registerLocalVariable(BuiltinVariables::DeviceName);
    for (auto i = 1; i <= s_fingerVariableCount; i++) {
        registerLocalVariable<QPointF>(QString("finger_%1_initial_position_percentage").arg(i));
        registerLocalVariable<QPointF>(QString("finger_%1_position_percentage").arg(i));
        registerLocalVariable<qreal>(QString("finger_%1_pressure").arg(i));
    }
    registerLocalVariable(BuiltinVariables::Fingers);
    registerRemoteVariable<Qt::KeyboardModifiers>(BuiltinVariables::KeyboardModifiers, [](auto &value) {
        value = g_keyboard->modifiers();
    });
    registerLocalVariable(BuiltinVariables::LastTriggerId);
    registerRemoteVariable<QPointF>("pointer_position_screen_percentage", [](auto &value) {
        value = g_pointerPositionGetter->screenPointerPosition();
    });
    registerRemoteVariable<QPointF>("pointer_position_window_percentage", [](auto &value) {
        const auto window = g_windowProvider->windowUnderPointer();
        if (!window) {
            return;
        }

        const auto windowGeometry = window->geometry();
        const auto pointerPos = g_pointerPositionGetter->globalPointerPosition();
        if (!pointerPos || !windowGeometry) {
            return;
        }
        const auto translatedPosition = pointerPos.value() - windowGeometry->topLeft();
        value = QPointF(translatedPosition.x() / windowGeometry->width(), translatedPosition.y() / windowGeometry->height());
    });
    registerLocalVariable(BuiltinVariables::ThumbInitialPositionPercentage);
    registerLocalVariable(BuiltinVariables::ThumbPositionPercentage);
    registerLocalVariable(BuiltinVariables::ThumbPresent);
    registerRemoteVariable<QString>("window_class", [](auto &value) {
        if (const auto window = g_windowProvider->activeWindow()) {
            value = window->resourceClass();
        }
    });
    registerRemoteVariable<bool>("window_fullscreen", [](auto &value) {
        if (const auto window = g_windowProvider->activeWindow()) {
            value = window->fullscreen();
        }
    });
    registerRemoteVariable<QString>("window_id", [](auto &value) {
        if (const auto window = g_windowProvider->activeWindow()) {
            value = window->id();
        }
    });
    registerRemoteVariable<bool>("window_maximized", [](auto &value) {
        if (const auto window = g_windowProvider->activeWindow()) {
            value = window->maximized();
        }
    });
    registerRemoteVariable<QString>("window_name", [](auto &value) {
        if (const auto window = g_windowProvider->activeWindow()) {
            value = window->resourceName();
        }
    });
    registerRemoteVariable<QString>("window_title", [](auto &value) {
        if (const auto window = g_windowProvider->activeWindow()) {
            value = window->title();
        }
    });
    registerRemoteVariable<QString>("window_under_class", [](auto &value) {
        if (const auto window = g_windowProvider->windowUnderPointer()) {
            value = window->resourceClass();
        }
    });
    registerRemoteVariable<bool>("window_under_fullscreen", [](auto &value) {
        if (const auto window = g_windowProvider->windowUnderPointer()) {
            value = window->fullscreen();
        }
    });
    registerRemoteVariable<QString>("window_under_id", [](auto &value) {
        if (const auto window = g_windowProvider->windowUnderPointer()) {
            value = window->id();
        }
    });
    registerRemoteVariable<bool>("window_under_maximized", [](auto &value) {
        if (const auto window = g_windowProvider->windowUnderPointer()) {
            value = window->maximized();
        }
    });
    registerRemoteVariable<QString>("window_under_name", [](auto &value) {
        if (const auto window = g_windowProvider->windowUnderPointer()) {
            value = window->resourceName();
        }
    });
    registerRemoteVariable<QString>("window_under_title", [](auto &value) {
        if (const auto window = g_windowProvider->windowUnderPointer()) {
            value = window->title();
        }
    });

    for (const auto &[name, variable] : m_variables) {
        if (variable->type() == typeid(QPointF)) {
            registerRemoteVariable<qreal>(name + "_x", [this, name](auto &value) {
                if (const auto point = getVariable<QPointF>(name)->get()) {
                    value = point->x();
                }
            });
            registerRemoteVariable<qreal>(name + "_y", [this, name](auto &value) {
                if (const auto point = getVariable<QPointF>(name)->get()) {
                    value = point->y();
                }
            });
        }
    }
}

VariableManager::~VariableManager() = default;

Variable *VariableManager::getVariable(const QString &name)
{
    if (!m_variables.contains(name)) {
        qCDebug(INPUTACTIONS_VARIABLE_MANAGER).noquote() << QString("Variable %1 not found").arg(name);
        return nullptr;
    }
    return m_variables.at(name).get();
}

void VariableManager::registerVariable(const QString &name, std::unique_ptr<Variable> variable)
{
    m_variables[name] = std::move(variable);
}

std::map<QString, const Variable *> VariableManager::variables() const
{
    std::map<QString, const Variable *> variables;
    for (const auto &[name, variable] : m_variables) {
        variables[name] = variable.get();
    }
    return variables;
}

}