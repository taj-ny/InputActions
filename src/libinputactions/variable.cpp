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

#include "variable.h"

#include <libinputactions/input/keyboard.h>
#include <libinputactions/input/pointer.h>
#include <libinputactions/window.h>

#include <QLoggingCategory>
#include <QPointF>
#include <QRegularExpression>

Q_LOGGING_CATEGORY(LIBINPUTACTIONS_VARIABLE, "libinputactions.variable", QtWarningMsg)

namespace libinputactions
{

VariableManager::VariableManager()
{
    registerRemoteVariable<CursorShape>("cursor_shape", [](auto &value) {
        value = Pointer::instance()->shape();
    });
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

template <typename T>
TypedVariable<T> *VariableManager::getVariable(const VariableInfo<T> &variable)
{
    return getVariable<T>(variable.name());
}

template <typename T>
TypedVariable<T> *VariableManager::getVariable(const QString &name)
{
    const auto variable = getVariable(name);
    return dynamic_cast<TypedVariable<T> *>(variable);
}

Variable *VariableManager::getVariable(const QString &name)
{
    if (!m_variables.contains(name)) {
        return nullptr;
    }
    return m_variables.at(name).get();
}

void VariableManager::registerVariable(const QString &name, std::unique_ptr<Variable> variable)
{
    m_variables[name] = std::move(variable);
}

template <typename T>
void VariableManager::registerLocalVariable(const QString &name)
{
    registerVariable(name, std::make_unique<LocalVariable<T>>());
}

template <typename T>
void VariableManager::registerRemoteVariable(const QString &name, const std::function<void(std::optional<T> &)> getter)
{
    registerVariable(name, std::make_unique<RemoteVariable<T>>(getter));
}

template TypedVariable<qreal> *VariableManager::getVariable(const VariableInfo<qreal> &variable);
template TypedVariable<Qt::MouseButtons> *VariableManager::getVariable(const VariableInfo<Qt::MouseButtons> &variable);

VariableManager *VariableManager::instance()
{
    return s_instance.get();
}

std::unique_ptr<VariableManager> VariableManager::s_instance = std::unique_ptr<VariableManager>(new VariableManager);

template <typename T>
VariableInfo<T>::VariableInfo(const QString &name)
    : m_name(name)
{
}

template <typename T>
const QString &VariableInfo<T>::name() const
{
    return m_name;
}

template class VariableInfo<bool>;
template class VariableInfo<Qt::KeyboardModifiers>;
template class VariableInfo<Qt::MouseButtons>;
template class VariableInfo<qreal>;
template class VariableInfo<QPointF>;
template class VariableInfo<QString>;

Variable::Variable(const std::type_index &type)
    : m_type(type)
{
}

bool Variable::compare(const std::vector<std::any> &values, const ComparisonOperator &comparisonOperator) const
{
    const auto left = getAny();
    if (!left.has_value()) {
        return false;
    }

    switch (comparisonOperator) {
        case ComparisonOperator::NotEqualTo:
            return !compare(left, values[0], ComparisonOperator::EqualTo);
        case ComparisonOperator::OneOf:
            return std::any_of(values.cbegin(), values.cend(), [this, &left](const auto &value) {
                return compare(left, value, ComparisonOperator::EqualTo);
            });
        case ComparisonOperator::Between:
            return values.size() >= 2 && compare(left, values[0], ComparisonOperator::GreaterThanOrEqual)
                   && compare(left, values[1], ComparisonOperator::LessThanOrEqual);
        default:
            return compare(left, values[0], comparisonOperator);
    }
}

bool Variable::compare(const std::any &left, const std::any &right, const ComparisonOperator &comparisonOperator) const
{
    return false;
}

template <typename T>
TypedVariable<T>::TypedVariable()
    : Variable(typeid(T))
{
}

template <typename T>
bool TypedVariable<T>::compare(const T &left, const T &right, const ComparisonOperator &comparisonOperator)
{
    return false;
}

template <>
bool TypedVariable<bool>::compare(const bool &left, const bool &right, const ComparisonOperator &comparisonOperator)
{
    switch (comparisonOperator) {
        case ComparisonOperator::EqualTo:
            return left == right;
        default:
            return false;
    }
}

template <>
bool TypedVariable<qreal>::compare(const double &left, const double &right, const ComparisonOperator &comparisonOperator)
{
    switch (comparisonOperator) {
        case ComparisonOperator::EqualTo:
            return left == right;
        case ComparisonOperator::GreaterThan:
            return left > right;
        case ComparisonOperator::GreaterThanOrEqual:
            return left >= right;
        case ComparisonOperator::LessThan:
            return left < right;
        case ComparisonOperator::LessThanOrEqual:
            return left <= right;
        case ComparisonOperator::NotEqualTo:
            return left != right;
        default:
            return false;
    }
}

#define TYPEDVARIABLE_COMPARE_QFLAGS(type)                                                                                                       \
    template <>                                                                                                                                  \
    bool TypedVariable<QFlags<type>>::compare(const QFlags<type> &left, const QFlags<type> &right, const ComparisonOperator &comparisonOperator) \
    {                                                                                                                                            \
        switch (comparisonOperator) {                                                                                                            \
            case ComparisonOperator::EqualTo:                                                                                                    \
                return left == right;                                                                                                            \
            case ComparisonOperator::Contains:                                                                                                   \
                return (left & right) == right;                                                                                                    \
            default:                                                                                                                             \
                return false;                                                                                                                    \
        }                                                                                                                                        \
    }

#define TYPEDVARIABLE_COMPARE_DEFAULT(type)                                                                              \
    template <>                                                                                                          \
    bool TypedVariable<type>::compare(const type &left, const type &right, const ComparisonOperator &comparisonOperator) \
    {                                                                                                                    \
        switch (comparisonOperator) {                                                                                    \
            case ComparisonOperator::EqualTo:                                                                            \
                return left == right;                                                                                    \
            default:                                                                                                     \
                return false;                                                                                            \
        }                                                                                                                \
    }

TYPEDVARIABLE_COMPARE_DEFAULT(CursorShape)
TYPEDVARIABLE_COMPARE_QFLAGS(Qt::KeyboardModifier)

template <>
bool TypedVariable<QPointF>::compare(const QPointF &left, const QPointF &right, const ComparisonOperator &comparisonOperator)
{
    return TypedVariable<qreal>::compare(left.x(), right.x(), comparisonOperator)
        && TypedVariable<qreal>::compare(left.y(), right.y(), comparisonOperator);
}

template <>
bool TypedVariable<QString>::compare(const QString &left, const QString &right, const ComparisonOperator &comparisonOperator)
{
    switch (comparisonOperator) {
        case ComparisonOperator::Contains:
            return left.contains(right);
        case ComparisonOperator::EqualTo:
            return left == right;
        case ComparisonOperator::Regex:
            return QRegularExpression(right).match(left).hasMatch();
        default:
            return false;
    }
}

template <typename T>
bool TypedVariable<T>::compare(const std::any &left, const std::any &right, const ComparisonOperator &comparisonOperator) const
{
    if (left.type() != typeid(T) || right.type() != typeid(T)) {
        qCWarning(LIBINPUTACTIONS_VARIABLE).noquote() << "Attempted illegal variable comparison (left: "
            << left.type().name() << ", right: " << right.type().name() << ", expected: " << typeid(T).name();
        return false;
    }
    return compare(std::any_cast<T>(left), std::any_cast<T>(right), comparisonOperator);
}

template <typename T>
std::optional<T> TypedVariable<T>::get() const
{
    return {};
}

template <typename T>
void TypedVariable<T>::set(const std::optional<T> &value)
{
}

template <typename T>
std::any TypedVariable<T>::getAny() const
{
    const auto value = get();
    if (value.has_value()) {
        return value.value();
    }
    return {};
}

template <typename T>
void TypedVariable<T>::setAny(const std::any &value)
{
    if (!value.has_value()) {
        set({});
        return;
    }
    set(std::any_cast<T>(value));
}

const std::type_index &Variable::type() const
{
    return m_type;
}

std::any Variable::getAny() const
{
    return {};
}

void Variable::setAny(const std::any &value)
{
}

template <typename T>
std::optional<T> LocalVariable<T>::get() const
{
    return m_value;
}

template <typename T>
void LocalVariable<T>::set(const std::optional<T> &value)
{
    m_value = value;
}

template <typename T>
RemoteVariable<T>::RemoteVariable(const std::function<void(std::optional<T> &value)> &getter)
    : TypedVariable<T>()
    , m_getter(getter)
{
}

template <typename T>
std::optional<T> RemoteVariable<T>::get() const
{
    std::optional<T> value;
    m_getter(value);
    return value;
}

}