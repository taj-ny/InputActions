#pragma once

#include "Test.h"

namespace InputActions
{

class Condition;
class Node;
class VariableCondition;

class TestNodeParser : public Test
{
    Q_OBJECT

private slots:
    void init();

    void triggerGroup__propertyAppliedToTrigger();
    void triggerGroup_nested_propertyInFirstGroup__propertyAppliedToTrigger();
    void triggerGroup_nested_propertyInSecondGroup__propertyAppliedToTrigger();

    void triggerGroup_condition_triggerWithoutCondition__conditionAppliedToTrigger();
    void triggerGroup_condition_triggerWithSingleCondition__conditionsMergedIntoGroup();
    void triggerGroup_nested_condition_triggerWithSingleCondition__conditionsMergedIntoGroup();
    void triggerGroup_condition_triggerWithAllConditionGroup__groupConditionPrependedIntoTriggerCondition();
    void triggerGroup_condition_triggerWithAnyConditionGroup__groupConditionAndTriggerConditionMergedIntoGroup();

private:
    static Node load(const QString &s);
};

}