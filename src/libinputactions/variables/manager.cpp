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

#include "manager.h"
#include "variable.h"

#include <libinputactions/input/keyboard.h>
#include <libinputactions/input/pointer.h>
#include <libinputactions/window.h>

#include <QLoggingCategory>
#include <QRegularExpression>

Q_LOGGING_CATEGORY(LIBINPUTACTIONS_VARIABLE_MANAGER, "libinputactions.variable.manager", QtWarningMsg)

namespace libinputactions
{

VariableManager::VariableManager()
{
    registerRemoteVariable<CursorShape>("cursor_shape", [](auto &value) {
        value = Pointer::instance()->shape();
    });
    for (auto i = 1; i <= s_fingerVariableCount; i ++) {
        registerLocalVariable<QPointF>(QString("finger_%1_position_percentage").arg(i));
    }
    registerLocalVariable<qreal>("fingers");
    registerRemoteVariable<Qt::KeyboardModifiers>("keyboard_modifiers", [](auto &value) {
        value = Keyboard::instance()->modifiers();
    });
    registerLocalVariable<Qt::MouseButtons>("mouse_buttons");
    registerRemoteVariable<QPointF>("pointer_position_screen_percentage", [](auto &value) {
        value = Pointer::instance()->screenPosition();
    });
    registerRemoteVariable<QPointF>("pointer_position_window_percentage", [](auto &value) {
        const auto window = WindowProvider::instance()->underPointer();
        const auto windowGeometry = window->geometry();
        const auto pointerPos = Pointer::instance()->globalPosition();
        if (!window || !pointerPos || !windowGeometry || !windowGeometry->contains(pointerPos.value())) {
            return;
        }

        const auto translatedPosition = pointerPos.value() - windowGeometry->topLeft();
        value = QPointF(translatedPosition.x() / windowGeometry->width(), translatedPosition.y() / windowGeometry->height());
    });
    registerRemoteVariable<QString>("window_class", [](auto &value) {
        if (const auto window = WindowProvider::instance()->active()) {
            value = window->resourceClass();
        }
    });
    registerRemoteVariable<bool>("window_fullscreen", [](auto &value) {
        if (const auto window = WindowProvider::instance()->active()) {
            value = window->fullscreen();
        }
    });
    registerRemoteVariable<QString>("window_id", [](auto &value) {
        if (const auto window = WindowProvider::instance()->active()) {
            value = window->id();
        }
    });
    registerRemoteVariable<bool>("window_maximized", [](auto &value) {
        if (const auto window = WindowProvider::instance()->active()) {
            value = window->maximized();
        }
    });
    registerRemoteVariable<QString>("window_name", [](auto &value) {
        if (const auto window = WindowProvider::instance()->active()) {
            value = window->resourceName();
        }
    });
    registerRemoteVariable<QString>("window_title", [](auto &value) {
        if (const auto window = WindowProvider::instance()->active()) {
            value = window->resourceClass();
        }
    });
    registerRemoteVariable<QString>("window_under_class", [](auto &value) {
        if (const auto window = WindowProvider::instance()->underPointer()) {
            value = window->resourceClass();
        }
    });
    registerRemoteVariable<bool>("window_under_fullscreen", [](auto &value) {
        if (const auto window = WindowProvider::instance()->underPointer()) {
            value = window->fullscreen();
        }
    });
    registerRemoteVariable<QString>("window_under_id", [](auto &value) {
        if (const auto window = WindowProvider::instance()->underPointer()) {
            value = window->id();
        }
    });
    registerRemoteVariable<bool>("window_under_maximized", [](auto &value) {
        if (const auto window = WindowProvider::instance()->underPointer()) {
            value = window->maximized();
        }
    });
    registerRemoteVariable<QString>("window_under_name", [](auto &value) {
        if (const auto window = WindowProvider::instance()->underPointer()) {
            value = window->resourceName();
        }
    });
    registerRemoteVariable<QString>("window_under_title", [](auto &value) {
        if (const auto window = WindowProvider::instance()->underPointer()) {
            value = window->resourceClass();
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

template<typename T>
std::optional<VariableWrapper<T>> VariableManager::getVariable(const VariableInfo<T> &variable)
{
    return getVariable<T>(variable.name());
}

template<typename T>
std::optional<VariableWrapper<T>> VariableManager::getVariable(const QString &name)
{
    auto *variable = getVariable(name);
    if (!variable) {
        return {};
    } else if (variable->type() != typeid(T)) {
        qCWarning(LIBINPUTACTIONS_VARIABLE_MANAGER).noquote() << QString("VariableManager::getVariable<T> called with the wrong type (variable: %1, type: %2")
            .arg(variable->type().name(), typeid(T).name());
        return {};
    }

    return VariableWrapper<T>(getVariable(name));
}

Variable *VariableManager::getVariable(const QString &name)
{
    if (!m_variables.contains(name)) {
        qCDebug(LIBINPUTACTIONS_VARIABLE_MANAGER).noquote() << QString("Variable %1 not found").arg(name);
        return nullptr;
    }
    return m_variables.at(name).get();
}

void VariableManager::registerVariable(const QString &name, std::unique_ptr<Variable> variable)
{
    m_variables[name] = std::move(variable);
}

template<typename T>
void VariableManager::registerLocalVariable(const QString &name)
{
    registerVariable(name, std::make_unique<LocalVariable>(typeid(T)));
}

template<typename T>
void VariableManager::registerRemoteVariable(const QString &name, const std::function<void(std::optional<T> &value)> getter)
{
    const std::function<void(std::any &value)> anyGetter = [getter](std::any &value) {
        std::optional<T> optValue;
        getter(optValue);
        if (optValue.has_value()) {
            value = optValue.value();
        }
    };
    registerVariable(name, std::make_unique<RemoteVariable>(typeid(T), anyGetter));
}

std::map<QString, const Variable *> VariableManager::variables() const
{
    std::map<QString, const Variable *> variables;
    for (const auto &[name, variable] : m_variables) {
        variables[name] = variable.get();
    }
    return variables;
}

VariableManager *VariableManager::instance()
{
    return s_instance.get();
}

std::unique_ptr<VariableManager> VariableManager::s_instance = std::unique_ptr<VariableManager>(new VariableManager);

template std::optional<VariableWrapper<QString>> VariableManager::getVariable(const QString &variable);
template std::optional<VariableWrapper<qreal>> VariableManager::getVariable(const VariableInfo<qreal> &variable);
template void VariableManager::registerLocalVariable<qreal>(const QString &name);

template<typename T>
VariableWrapper<T>::VariableWrapper(Variable *variable)
    : m_variable(variable)
{
}

template<typename T>
std::optional<T> VariableWrapper<T>::get() const
{
    const auto value = m_variable->get();
    if (!value.has_value()) {
        return {};
    }
    return std::any_cast<T>(value);
}

template<typename T>
void VariableWrapper<T>::set(const std::optional<T> &value)
{
    if (!value.has_value()) {
        m_variable->set({});
        return;
    }
    m_variable->set(value.value());
}

template class VariableWrapper<qreal>;
template class VariableWrapper<QPointF>;
template class VariableWrapper<QString>;

template<typename T>
VariableInfo<T>::VariableInfo(const QString &name)
    : m_name(name)
{
}

template<typename T>
const QString &VariableInfo<T>::name() const
{
    return m_name;
}

template class VariableInfo<Qt::KeyboardModifiers>;
template class VariableInfo<qreal>;

}