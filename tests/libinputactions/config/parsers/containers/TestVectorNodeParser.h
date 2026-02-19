#pragma once

#include "Test.h"

namespace InputActions
{

class TestVectorNodeParser : public Test
{
    Q_OBJECT

private slots:
    void valid__parsesNodeCorrectly();
    void duplicateItem__parsesNodeCorrectly();

    void invalid_scalar__throwsInvalidNodeTypeConfigException();
};

}