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

#pragma once

#include "Condition.h"
#include <QString>

namespace libinputactions
{

enum class ComparisonOperator;

/**
 * A condition that is constructed right before evaluation. If the construction fails, the condition fails to evaluate and construction will be attempted again
 * on further evaluations.
 */
class LazyCondition : public Condition
{
public:
    LazyCondition(std::function<std::shared_ptr<Condition>()> constructor, QString errorMessage = "");

protected:
    ConditionEvaluationResult evaluateImpl() override;

private:
    std::function<std::shared_ptr<Condition>()> m_constructor;
    std::shared_ptr<Condition> m_condition;

    QString m_errorMessage;
    bool m_errorNotificationShown{};
};

}