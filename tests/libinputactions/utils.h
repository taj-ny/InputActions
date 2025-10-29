#pragma once

#include <memory>

namespace libinputactions
{

class Condition;
enum class ConditionEvaluationResult;

extern std::shared_ptr<Condition> ERROR_CONDITION;
extern std::shared_ptr<Condition> FALSE_CONDITION;
extern std::shared_ptr<Condition> TRUE_CONDITION;

std::shared_ptr<Condition> referenceCondition(bool &result);

}