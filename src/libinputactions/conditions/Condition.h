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

namespace InputActions
{

class VariableManager;

struct ConditionEvaluationArguments
{
    ConditionEvaluationArguments();

    /**
     * Set to the global manager by default.
     */
    VariableManager *variableManager;
};

class Condition
{
public:
    virtual ~Condition() = default;

    /**
     * @returns True if the condition is satisfied, false if not or an exception is thrown.
     */
    bool satisfied(const ConditionEvaluationArguments &arguments = {});
    /**
     * @throws std::exception Evaluation of the condition failed.
     * @returns Whether the condition is satisfied.
     */
    bool evaluate(const ConditionEvaluationArguments &arguments = {});

    void setNegate(bool value) { m_negate = value; }

protected:
    Condition() = default;

    /**
     * @see evaluate
     */
    virtual bool evaluateImpl(const ConditionEvaluationArguments &arguments);

private:
    bool m_exceptionNotificationShown{};
    bool m_negate{};
};

}