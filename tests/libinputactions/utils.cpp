#include "utils.h"
#include <libinputactions/conditions/CustomCondition.cpp>

namespace libinputactions
{

std::shared_ptr<Condition> ERROR_CONDITION = std::make_shared<CustomCondition>([](const auto &arguments) -> bool {
    throw std::runtime_error("test error");
});
std::shared_ptr<Condition> FALSE_CONDITION = std::make_shared<CustomCondition>([](const auto &arguments) {
    return false;
});
std::shared_ptr<Condition> TRUE_CONDITION = std::make_shared<CustomCondition>([](const auto &arguments) {
    return true;
    ;
});

std::shared_ptr<Condition> referenceCondition(bool &result)
{
    return std::make_shared<CustomCondition>([&result](const auto &arguments) {
        return result;
    });
}

}