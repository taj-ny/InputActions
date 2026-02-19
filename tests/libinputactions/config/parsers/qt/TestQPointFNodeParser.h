#pragma once

#include "Test.h"

namespace InputActions
{

class TestQPointFNodeParser : public Test
{
    Q_OBJECT

private slots:
    void valid__parsesNodeCorrectly();
    void invalid__throwsInvalidValueConfigException();
};

}