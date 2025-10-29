#pragma once

#include "Test.h"

namespace libinputactions
{

class TestConditionGroup : public Test
{
    Q_OBJECT

private slots:
    void evaluate_data();
    void evaluate();
};

}