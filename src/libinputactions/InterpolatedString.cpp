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

#include "InterpolatedString.h"
#include <QRegularExpression>
#include <libinputactions/variables/VariableManager.h>

namespace libinputactions
{

InterpolatedString::InterpolatedString(QString string)
    : m_string(std::move(string))
{
    static const QRegularExpression variableReferenceRegex("\\$([a-zA-Z0-9_])+");
    const auto variables = g_variableManager->variables();
    auto it = variableReferenceRegex.globalMatch(m_string);
    while (it.hasNext()) {
        const auto match = it.next();
        const auto variableName = match.captured(0).mid(1);
        if (!variables.contains(variableName)) {
            continue;
        }
        m_variables.insert(variableName);
    }
}

InterpolatedString::operator Value<QString>() const
{
    return Value<QString>::function([string = *this] {
        return string.evaluate();
    });
}

QString InterpolatedString::evaluate() const
{
    if (m_variables.empty()) {
        return m_string;
    }

    QString result = m_string;
    for (const auto &variable : m_variables) {
        const auto value = g_variableManager->getVariable(variable)->operations()->toString();
        result = result.replace(QRegularExpression("\\$" + variable + "(?![a-zA-Z0-9_])"), value);
    }
    return result;
}

}