#pragma once

#include "Test.h"

namespace InputActions
{

class TestTriggerGroupNodeParser : public Test
{
    Q_OBJECT

private slots:
    void init();

    void oneProperty__parsesNodeCorrectly();
    void twoTriggers__parsesNodeCorrectly();
    void nested_propertyInFirstGroup__parsesNodeCorrectly();
    void nested_propertyInSecondGroup__parsesNodeCorrectly();

    void condition_triggerWithoutCondition__appliesConditionToTrigger();
    void condition_triggerWithSingleCondition__mergesConditionsIntoAllGroup();
    void nested_condition_triggerWithSingleCondition__mergesConditionsIntoAllGroup();
    void condition_triggerWithAllConditionGroup__prependsConditionToTrigger();
    void condition_triggerWithAnyConditionGroup__mergesConditionsIntoAllGroup();

    void invalidPropertyValue__throwsExceptionAtCorrectPosition();
    void unusedProperty__addsConfigIssueAtCorrectPosition();
};

}