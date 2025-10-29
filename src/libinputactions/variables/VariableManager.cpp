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

Q_LOGGING_CATEGORY(INPUTACTIONS_VARIABLE_MANAGER, "inputactions.variable.manager", QtWarningMsg)

namespace InputActions
{

VariableManager::VariableManager() = default;
VariableManager::~VariableManager() = default;

bool VariableManager::hasVariable(const QString &name) const
{
    return m_variables.contains(name);
}

Variable *VariableManager::getVariable(const QString &name) const
{
    if (!m_variables.contains(name)) {
        qCDebug(INPUTACTIONS_VARIABLE_MANAGER).noquote() << QString("Variable %1 not found").arg(name);
        return nullptr;
    }
    return m_variables.at(name).get();
}

Variable *VariableManager::registerVariable(const QString &name, std::unique_ptr<Variable> variable, bool hidden)
{
    variable->m_hidden = hidden;
    m_variables[name] = std::move(variable);

    if (m_variables[name]->type() == typeid(QPointF)) {
        registerRemoteVariable<qreal>(
            name + "_x",
            [this, name](auto &value) {
                if (const auto point = getVariable<QPointF>(name)->get()) {
                    value = point->x();
                }
            },
            true);
        registerRemoteVariable<qreal>(
            name + "_y",
            [this, name](auto &value) {
                if (const auto point = getVariable<QPointF>(name)->get()) {
                    value = point->y();
                }
            },
            true);
    }

    return m_variables[name].get();
}

void VariableManager::setProcessEnvironment(QProcess &process) const
{
    auto environment = QProcessEnvironment::systemEnvironment();
    for (const auto &argument : process.arguments()) {
        static const QRegularExpression variableReferenceRegex("\\$([a-zA-Z0-9_])+");
        auto it = variableReferenceRegex.globalMatch(argument);
        while (it.hasNext()) {
            const auto match = it.next();
            const auto variableName = match.captured(0).mid(1);

            if (const auto *variable = getVariable(variableName)) {
                const auto value = variable->get();
                if (!value.has_value()) {
                    continue;
                }

                if (variable->type() == typeid(bool)) {
                    if (std::any_cast<bool>(value)) {
                        environment.insert(variableName, "1");
                    }
                    continue;
                }

                environment.insert(variableName, variable->operations()->toString(value));
            }
        }
    }
    process.setProcessEnvironment(environment);
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