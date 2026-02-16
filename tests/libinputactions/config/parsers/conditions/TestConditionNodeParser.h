#pragma once

#include "Test.h"

namespace InputActions
{

class TestConditionNodeParser : public Test
{
    Q_OBJECT

private slots:
    void invalid_map__throwsInvalidValueConfigException();
    void invalid_scalar__throwsInvalidValueConfigException();
};

}