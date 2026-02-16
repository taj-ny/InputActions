#pragma once

#include "Test.h"

namespace InputActions
{

class TestEnumNodeParser : public Test
{
    Q_OBJECT

private slots:
    void valid__parsesNodeCorrectly();
    void invalid_differentCase__throwsInvalidValueConfigException();
    void invalid__throwsInvalidValueConfigException();
};

}