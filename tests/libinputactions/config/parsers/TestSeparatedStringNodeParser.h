#pragma once

#include "Test.h"

namespace InputActions
{

class TestSeparatedStringNodeParser : public Test
{
    Q_OBJECT

private slots:
    void number__parsesNodeCorrectly();
    void point__parsesNodeCorrectly();
    void string__parsesNodeCorrectly();

    void invalid__throwsInvalidValueConfigException_data();
    void invalid__throwsInvalidValueConfigException();
};

}