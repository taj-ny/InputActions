#pragma once

#include <libinputactions/conditions/CallbackCondition.h>

#include <memory>

namespace libinputactions
{

std::shared_ptr<Condition> makeCondition(const bool &result)
{
    return std::make_shared<CallbackCondition>([result]() { return result; });
}

}
