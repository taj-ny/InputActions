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

#include "QVariant.h"
#include <QSizeF>
#include <QStringList>

namespace InputActions::QVariantHelpers
{

QString toString(const QVariant &variant)
{
    const auto userType = variant.userType();
    switch (userType) {
        case QMetaType::QSizeF: {
            const auto value = variant.toSizeF();
            return QString("%1,%2").arg(QString::number(value.width()), QString::number(value.height()));
        }
        default:
            if (userType == qMetaTypeId<std::chrono::milliseconds>()) {
                const auto value = variant.value<std::chrono::milliseconds>();
                return QString("%1 ms").arg(QString::number(value.count()));
            }
            break;
    }

    return variant.toString();
}

}