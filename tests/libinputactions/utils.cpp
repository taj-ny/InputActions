#include "utils.h"
#include <libinputactions/conditions/CustomCondition.cpp>

namespace libinputactions
{

std::shared_ptr<Condition> ERROR_CONDITION = std::make_shared<CustomCondition>([]() {
    return ConditionEvaluationResult::Error;
});
std::shared_ptr<Condition> FALSE_CONDITION = std::make_shared<CustomCondition>([]() {
    return ConditionEvaluationResult::NotSatisfied;
});
std::shared_ptr<Condition> TRUE_CONDITION = std::make_shared<CustomCondition>([]() {
    return ConditionEvaluationResult::Satisfied;
});

std::shared_ptr<Condition> referenceCondition(ConditionEvaluationResult &result)
{
    return std::make_shared<CustomCondition>([&result]() {
        return result;
    });
}

}