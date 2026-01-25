#include "TestNodeParser.h"
#include "libinputactions/config/ConfigIssueManager.h"
#include <libinputactions/actions/InputAction.h>
#include <libinputactions/conditions/ConditionGroup.h>
#include <libinputactions/conditions/VariableCondition.h>
#include <libinputactions/config/Node.h>
#include <libinputactions/globals.h>
#include <libinputactions/triggers/Trigger.h>
#include <libinputactions/variables/VariableManager.h>
#include <memory>
#include <qtestcase.h>

namespace InputActions
{

void TestNodeParser::init()
{
    g_variableManager->registerLocalVariable<bool>("a");
    g_variableManager->registerLocalVariable<bool>("b");
    g_variableManager->registerLocalVariable<bool>("c");
    g_variableManager->registerLocalVariable<bool>("d");
}

void TestNodeParser::triggerGroup__propertyAppliedToTrigger()
{
    const auto node = load(R"(
        - id: test
          gestures:
            - type: press
    )");

    const auto triggers = node.as<std::vector<std::unique_ptr<Trigger>>>();
    QCOMPARE(triggers[0]->id(), "test");
}

void TestNodeParser::triggerGroup_nested_propertyInFirstGroup__propertyAppliedToTrigger()
{
    const auto node = load(R"(
        - id: test
          gestures:
            - gestures:
                - type: press
    )");

    const auto triggers = node.as<std::vector<std::unique_ptr<Trigger>>>();
    QCOMPARE(triggers[0]->id(), "test");
}

void TestNodeParser::triggerGroup_nested_propertyInSecondGroup__propertyAppliedToTrigger()
{
    const auto node = load(R"(
        - gestures:
            - id: test
              gestures:
                - type: press
    )");

    const auto triggers = node.as<std::vector<std::unique_ptr<Trigger>>>();
    QCOMPARE(triggers[0]->id(), "test");
}

void TestNodeParser::triggerGroup_condition_triggerWithoutCondition__conditionAppliedToTrigger()
{
    const auto node = load(R"(
        - conditions: $a
          gestures:
            - type: press
    )");

    const auto triggers = node.as<std::vector<std::unique_ptr<Trigger>>>();
    Q_ASSERT(dynamic_pointer_cast<VariableCondition>(triggers[0]->activationCondition()));
}

void TestNodeParser::triggerGroup_condition_triggerWithSingleCondition__conditionsMergedIntoGroup()
{
    const auto node = load(R"(
        - conditions: $a
          gestures:
            - type: press
              conditions: $b
    )");

    const auto triggers = node.as<std::vector<std::unique_ptr<Trigger>>>();
    const auto condition = std::dynamic_pointer_cast<ConditionGroup>(triggers[0]->activationCondition());

    Q_ASSERT(condition);
    QCOMPARE(condition->mode(), ConditionGroupMode::All);

    const auto &conditionGroupItems = condition->conditions();
    QCOMPARE(conditionGroupItems.size(), 2);

    QCOMPARE(dynamic_pointer_cast<VariableCondition>(conditionGroupItems[0])->variableName(), "a");
    QCOMPARE(dynamic_pointer_cast<VariableCondition>(conditionGroupItems[1])->variableName(), "b");
}

void TestNodeParser::triggerGroup_nested_condition_triggerWithSingleCondition__conditionsMergedIntoGroup()
{
    const auto node = load(R"(
        - conditions: $a
          gestures:
            - conditions: $b
              gestures:
                - type: press
                  conditions: $c
    )");

    const auto triggers = node.as<std::vector<std::unique_ptr<Trigger>>>();
    const auto condition = std::dynamic_pointer_cast<ConditionGroup>(triggers[0]->activationCondition());

    const auto &conditionGroupItems = condition->conditions();

    QCOMPARE(conditionGroupItems.size(), 3);
    QCOMPARE(dynamic_pointer_cast<VariableCondition>(conditionGroupItems[0])->variableName(), "a");
    QCOMPARE(dynamic_pointer_cast<VariableCondition>(conditionGroupItems[1])->variableName(), "b");
    QCOMPARE(dynamic_pointer_cast<VariableCondition>(conditionGroupItems[2])->variableName(), "c");
}

void TestNodeParser::triggerGroup_condition_triggerWithAllConditionGroup__groupConditionPrependedIntoTriggerCondition()
{
    const auto node = load(R"(
        - conditions: $a
          gestures:
            - type: press
              conditions:
                - $b
                - $c
    )");

    const auto triggers = node.as<std::vector<std::unique_ptr<Trigger>>>();
    const auto condition = triggers[0]->activationCondition();

    const auto conditionGroup = std::dynamic_pointer_cast<ConditionGroup>(condition);
    const auto &conditionGroupItems = conditionGroup->conditions();

    QCOMPARE(conditionGroupItems.size(), 3);
    QCOMPARE(dynamic_pointer_cast<VariableCondition>(conditionGroupItems[0])->variableName(), "a");
    QCOMPARE(dynamic_pointer_cast<VariableCondition>(conditionGroupItems[1])->variableName(), "b");
    QCOMPARE(dynamic_pointer_cast<VariableCondition>(conditionGroupItems[2])->variableName(), "c");
}

void TestNodeParser::triggerGroup_condition_triggerWithAnyConditionGroup__groupConditionAndTriggerConditionMergedIntoGroup()
{
    const auto node = load(R"(
        - conditions: $a
          gestures:
            - type: press
              conditions:
                any:
                  - $b
                  - $c
    )");

    const auto triggers = node.as<std::vector<std::unique_ptr<Trigger>>>();
    const auto condition = triggers[0]->activationCondition();

    const auto conditionGroup = std::dynamic_pointer_cast<ConditionGroup>(condition);
    QCOMPARE(conditionGroup->mode(), ConditionGroupMode::All);

    const auto &conditionGroupItems = conditionGroup->conditions();
    QCOMPARE(conditionGroupItems.size(), 2);

    QCOMPARE(dynamic_pointer_cast<VariableCondition>(conditionGroupItems[0])->variableName(), "a");

    const auto conditionGroup2 = std::dynamic_pointer_cast<ConditionGroup>(conditionGroupItems[1]);
    const auto &conditionGroup2Items = conditionGroup2->conditions();
    QCOMPARE(conditionGroup2->mode(), ConditionGroupMode::Any);

    QCOMPARE(dynamic_pointer_cast<VariableCondition>(conditionGroup2Items[0])->variableName(), "b");
    QCOMPARE(dynamic_pointer_cast<VariableCondition>(conditionGroup2Items[1])->variableName(), "c");
}

Node TestNodeParser::load(const QString &s)
{
    return Node(YAML::Load(s.toStdString()));
}

}

QTEST_MAIN(InputActions::TestNodeParser)
#include "TestNodeParser.moc"