#pragma once

#include "Test.h"

namespace InputActions
{

class TestQStringListNodeParser : public Test
{
    Q_OBJECT

private slots:
    void valid__parsesNodeCorrectly();
};

}