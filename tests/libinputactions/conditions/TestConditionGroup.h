#pragma once

#include "Test.h"

namespace InputActions
{

class TestConditionGroup : public Test
{
    Q_OBJECT

private slots:
    void evaluate_data();
    void evaluate();
};

}