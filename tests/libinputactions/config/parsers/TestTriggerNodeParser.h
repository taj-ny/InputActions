#pragma once

#include "Test.h"

namespace InputActions
{

class TestTriggerNodeParser : public Test
{
    Q_OBJECT

private slots:
    void noType__throwsConfigParserException();
    void invalidType__throwsConfigParserException();

    void stroke_withBeginAction__throwsConfigParserException();
    void stroke_withEndAction__doesNotThrowConfigParserException();

    void fingers__conditionCorrectlyConstructed();
    void fingers_range__conditionCorrectlyConstructed();
    void fingers_triggerCondition__mergedIntoAllGroup();
};

}