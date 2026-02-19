#pragma once

#include "Test.h"

namespace InputActions
{

class TestRangeNodeParser : public Test
{
    Q_OBJECT

private slots:
    void valid__parsesNodeCorrectly();
    void singularValue__parsesNodeCorrectly();
};

}