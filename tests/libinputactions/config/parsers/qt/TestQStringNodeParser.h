#pragma once

#include "Test.h"

namespace InputActions
{

class TestQStringNodeParser : public Test
{
    Q_OBJECT

private slots:
    void valid__parsesNodeCorrectly();
};

}