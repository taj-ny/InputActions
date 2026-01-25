#pragma once

#include "Test.h"

namespace InputActions
{

class TestContainerNodeParser : public Test
{
    Q_OBJECT

private slots:
    void set__parsedCorrectly();
    void set__duplicateElement__throwsDuplicateSetItemConfigException();
    void set__scalar__throwsInvalidNodeTypeConfigException();

    void vector__parsedCorrectly();
    void vector__duplicateElement__parsedCorrectly();
    void vector__scalar__throwsInvalidNodeTypeConfigException();
};

}