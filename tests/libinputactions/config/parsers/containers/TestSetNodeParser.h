#pragma once

#include "Test.h"

namespace InputActions
{

class TestSetNodeParser : public Test
{
    Q_OBJECT

private slots:
    void valid__parsesNodeCorrectly();

    void invalid_duplicateItem__throwsDuplicateSetItemConfigException();
    void invalid_scalar__throwsInvalidNodeTypeConfigException();
};

}