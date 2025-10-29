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

#include "VariableOperations.h"
#include "Variable.h"
#include <QLocale>
#include <QLoggingCategory>
#include <QPointF>
#include <QRegularExpression>
#include <any>
#include <libinputactions/interfaces/CursorShapeProvider.h>

Q_LOGGING_CATEGORY(INPUTACTIONS_VARIABLE_OPERATIONS, "inputactions.variable.operations")

namespace InputActions
{

VariableOperationsBase::VariableOperationsBase(Variable *variable)
    : m_variable(variable)
{
}

bool VariableOperationsBase::compare(const std::vector<std::any> &right, ComparisonOperator comparisonOperator) const
{
    const auto left = m_variable->get();
    if (!left.has_value()) {
        return false;
    }

    switch (comparisonOperator) {
        case ComparisonOperator::NotEqualTo:
            return !compare(left, right[0], ComparisonOperator::EqualTo);
        case ComparisonOperator::OneOf:
            return std::any_of(right.cbegin(), right.cend(), [this, &left](const auto &value) {
                return compare(left, value, ComparisonOperator::EqualTo);
            });
        case ComparisonOperator::Between:
            return right.size() >= 2 && compare(left, right[0], ComparisonOperator::GreaterThanOrEqual)
                && compare(left, right[1], ComparisonOperator::LessThanOrEqual);
        default:
            return compare(left, right[0], comparisonOperator);
    }
}

bool VariableOperationsBase::compare(const std::any &left, const std::any &right, ComparisonOperator comparisonOperator) const
{
    return false;
}

QString VariableOperationsBase::toString() const
{
    return toString(m_variable->get());
}

QString VariableOperationsBase::toString(const std::any &value) const
{
    return {};
}

std::unique_ptr<VariableOperationsBase> VariableOperationsBase::create(Variable *variable)
{
    const auto type = variable->type();
    if (type == typeid(bool)) {
        return std::make_unique<VariableOperations<bool>>(variable);
    } else if (type == typeid(qreal)) {
        return std::make_unique<VariableOperations<qreal>>(variable);
    } else if (type == typeid(CursorShape)) {
        return std::make_unique<VariableOperations<CursorShape>>(variable);
    } else if (type == typeid(Qt::KeyboardModifiers)) {
        return std::make_unique<VariableOperations<Qt::KeyboardModifiers>>(variable);
    } else if (type == typeid(InputDeviceTypes)) {
        return std::make_unique<VariableOperations<InputDeviceTypes>>(variable);
    } else if (type == typeid(Qt::MouseButtons)) {
        return std::make_unique<VariableOperations<Qt::MouseButtons>>(variable);
    } else if (type == typeid(QPointF)) {
        return std::make_unique<VariableOperations<QPointF>>(variable);
    } else if (type == typeid(QString)) {
        return std::make_unique<VariableOperations<QString>>(variable);
    }

    qCDebug(INPUTACTIONS_VARIABLE_OPERATIONS).noquote() << "No variable operations for type " << type.name();
    return {};
}

template<typename T>
VariableOperations<T>::VariableOperations(Variable *variable)
    : VariableOperationsBase(variable)
{
}

template<typename T>
bool VariableOperations<T>::compare(const std::any &left, const std::any &right, ComparisonOperator comparisonOperator) const
{
    if (left.type() != typeid(T) || right.type() != typeid(T)) {
        qCWarning(INPUTACTIONS_VARIABLE_OPERATIONS).noquote() << "Attempted illegal variable comparison (left: " << left.type().name()
                                                              << ", right: " << right.type().name() << ", expected: " << typeid(T).name();
        return false;
    }
    return compare(std::any_cast<T>(left), std::any_cast<T>(right), comparisonOperator);
}

template<>
bool VariableOperations<qreal>::compare(const qreal &left, const qreal &right, ComparisonOperator comparisonOperator)
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

#define VARIABLEOPERATIONS_COMPARE_QFLAGS(type)                                                                                                \
    template<>                                                                                                                                 \
    bool VariableOperations<QFlags<type>>::compare(const QFlags<type> &left, const QFlags<type> &right, ComparisonOperator comparisonOperator) \
    {                                                                                                                                          \
        switch (comparisonOperator) {                                                                                                          \
            case ComparisonOperator::EqualTo:                                                                                                  \
                return left == right;                                                                                                          \
            case ComparisonOperator::Contains:                                                                                                 \
                return (left & right) == right;                                                                                                \
            default:                                                                                                                           \
                return false;                                                                                                                  \
        }                                                                                                                                      \
    }                                                                                                                                          \
                                                                                                                                               \
    template<>                                                                                                                                 \
    QString VariableOperations<QFlags<type>>::toString(const QFlags<type> &value)                                                              \
    {                                                                                                                                          \
        QString s;                                                                                                                             \
        QDebug{&s} << value;                                                                                                                   \
        return s;                                                                                                                              \
    }

VARIABLEOPERATIONS_COMPARE_QFLAGS(Qt::KeyboardModifier)
VARIABLEOPERATIONS_COMPARE_QFLAGS(InputDeviceType)

template<>
bool VariableOperations<QPointF>::compare(const QPointF &left, const QPointF &right, ComparisonOperator comparisonOperator)
{
    return VariableOperations<qreal>::compare(left.x(), right.x(), comparisonOperator)
        && VariableOperations<qreal>::compare(left.y(), right.y(), comparisonOperator);
}

template<>
bool VariableOperations<QString>::compare(const QString &left, const QString &right, ComparisonOperator comparisonOperator)
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

template<>
QString VariableOperations<bool>::toString(const bool &value)
{
    return value ? "true" : "false";
}

template<>
QString VariableOperations<qreal>::toString(const qreal &value)
{
    return QString::number(value, 'f', QLocale::FloatingPointShortest);
}

template<>
QString VariableOperations<CursorShape>::toString(const CursorShape &value)
{
    for (const auto &[name, shape] : CURSOR_SHAPES) {
        if (shape == value) {
            return name;
        }
    }
    return "<unknown>";
}

template<>
QString VariableOperations<QPointF>::toString(const QPointF &value)
{
    return QString("(%1, %2)").arg(value.x()).arg(value.y());
}

template<>
QString VariableOperations<QString>::toString(const QString &value)
{
    return value;
}

template<typename T>
QString VariableOperations<T>::toString(const T &value)
{
    return "<toString operation not defined>";
}

template<typename T>
QString VariableOperations<T>::toString(const std::any &value) const
{
    if (!value.has_value()) {
        return "<null>";
    }
    return toString(std::any_cast<T>(value));
}

template<typename T>
bool VariableOperations<T>::compare(const T &left, const T &right, ComparisonOperator comparisonOperator)
{
    switch (comparisonOperator) {
        case ComparisonOperator::EqualTo:
            return left == right;
        default:
            return false;
    }
}

}