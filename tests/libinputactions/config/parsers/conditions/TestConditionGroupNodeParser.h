#pragma once

#include "Test.h"

namespace InputActions
{

class TestConditionGroupNodeParser : public Test
{
    Q_OBJECT

private slots:
    void all__parsesNodeCorrectly();
    void any__parsesNodeCorrectly();
    void none__parsesNodeCorrectly();

    void list__parsesNodeAsAllGroup();
    void nested__parsesNodeCorrectly();

    void invalid_scalarAsChild__throwsInvalidNodeTypeConfigException();
};

}