#pragma once

#include "Test.h"

namespace InputActions
{

class TestFlagsNodeParser : public Test
{
    Q_OBJECT

private slots:
    void empty__parsesNodeCorrectly();
    void oneItem__parsesNodeCorrectly();
    void twoItems__parsesNodeCorrectly();

    void invalid_duplicateItem__throwsInvalidNodeTypeConfigException();
    void invalid_scalar__throwsInvalidNodeTypeConfigException();
};

}