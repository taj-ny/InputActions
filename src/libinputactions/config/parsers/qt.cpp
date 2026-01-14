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

#include <libinputactions/config/Config.h>
#include <libinputactions/config/Node.h>
#include "containers.h"
#include "NodeParser.h"
#include <QPointF>
#include <QRegularExpression>
#include <QString>
#include <QStringList>

namespace InputActions
{

template<>
void NodeParser<QPointF>::parse(const Node *node, QPointF &result)
{
    const auto raw = node->as<QString>().split(",");
    if (raw.size() != 2) {
        throw ConfigParserException(node, "Invalid point, format: x,y (where both values are floats).");
    }

    bool ok{};
    const auto x = raw[0].toDouble(&ok);
    if (!ok) {
        throw ConfigParserException(node, "Failed to parse X value as a number.");
    }

    const auto y = raw[1].toDouble(&ok);
    if (!ok) {
        throw ConfigParserException(node, "Failed to parse Y value as a number.");
    }

    result.setX(x);
    result.setY(y);
}

template<>
void NodeParser<QRegularExpression>::parse(const Node *node, QRegularExpression &result)
{
    result = QRegularExpression(node->as<QString>());
    if (!result.isValid()) {
        throw ConfigParserException(node, "Invalid regular expression: " + result.errorString());
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
    for (auto string : node->as<std::vector<QString>>()) {
        result << string;
    }
}

}