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
#include "containers.h"
#include <QPointF>
#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <libinputactions/config/ConfigIssue.h>
#include <libinputactions/config/Node.h>

namespace InputActions
{

template<>
void NodeParser<QPointF>::parse(const Node *node, QPointF &result)
{
    const auto raw = node->as<QString>().split(",");
    if (raw.size() != 2) {
        throw InvalidValueConfigException(node->position(), "Invalid point, format: x,y (where both values are floats).");
    }

    bool ok{};
    const auto x = raw[0].toDouble(&ok);
    if (!ok) {
        throw InvalidValueConfigException(node->position(), QString("Value '%1' is not a number.").arg(raw[0]));
    }

    const auto y = raw[1].toDouble(&ok);
    if (!ok) {
        throw InvalidValueConfigException(node->position(), QString("Value '%1' is not a number.").arg(raw[1]));
    }

    result.setX(x);
    result.setY(y);
}

template<>
void NodeParser<QRegularExpression>::parse(const Node *node, QRegularExpression &result)
{
    result = QRegularExpression(node->as<QString>());
    if (!result.isValid()) {
        throw InvalidValueConfigException(node->position(), QString("Invalid regular expression: %1.").arg( result.errorString()));
    }
}

template<>
void NodeParser<QString>::parse(const Node *node, QString &result)
{
    result = QString::fromStdString(node->as<std::string>());
}

template<>
void NodeParser<QStringList>::parse(const Node *node, QStringList &result)
{
    for (const auto &s : node->as<std::vector<QString>>()) {
        result << s;
    }
}

}