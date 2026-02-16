#include "TestTriggerGroupNodeParser.h"
#include <libinputactions/conditions/ConditionGroup.h>
#include <libinputactions/conditions/VariableCondition.h>
#include <libinputactions/config/ConfigIssue.h>
#include <libinputactions/config/ConfigIssueManager.h>
#include <libinputactions/config/Node.h>
#include <libinputactions/triggers/Trigger.h>
#include <libinputactions/variables/VariableManager.h>

namespace InputActions
{

void TestTriggerGroupNodeParser::init()
{
    g_variableManager = std::make_shared<VariableManager>();
    g_variableManager->registerLocalVariable<bool>("a");
    g_variableManager->registerLocalVariable<bool>("b");
    g_variableManager->registerLocalVariable<bool>("c");
    g_variableManager->registerLocalVariable<bool>("d");
}

void TestTriggerGroupNodeParser::oneProperty__parsesNodeCorrectly()
{
    const auto node = Node::create(R"(
        - id: test
          gestures:
            - type: press
    )");
    const auto triggers = node->as<std::vector<std::unique_ptr<Trigger>>>();

    QCOMPARE(triggers.size(), 1);
    QCOMPARE(triggers[0]->id(), "test");
}

void TestTriggerGroupNodeParser::twoTriggers__parsesNodeCorrectly()
{
    const auto node = Node::create(R"(
        - id: test
          gestures:
            - type: press
            - type: press
    )");
    const auto triggers = node->as<std::vector<std::unique_ptr<Trigger>>>();

    QCOMPARE(triggers.size(), 2);
    QCOMPARE(triggers[0]->id(), "test");
    QCOMPARE(triggers[1]->id(), "test");
}

void TestTriggerGroupNodeParser::nested_propertyInFirstGroup__parsesNodeCorrectly()
{
    const auto node = Node::create(R"(
        - id: test
          gestures:
            - gestures:
                - type: press
    )");
    const auto triggers = node->as<std::vector<std::unique_ptr<Trigger>>>();

    QCOMPARE(triggers.size(), 1);
    QCOMPARE(triggers[0]->id(), "test");
}

void TestTriggerGroupNodeParser::nested_propertyInSecondGroup__parsesNodeCorrectly()
{
    const auto node = Node::create(R"(
        - gestures:
            - id: test
              gestures:
                - type: press
    )");
    const auto triggers = node->as<std::vector<std::unique_ptr<Trigger>>>();

    QCOMPARE(triggers.size(), 1);
    QCOMPARE(triggers[0]->id(), "test");
}

void TestTriggerGroupNodeParser::condition_triggerWithoutCondition__appliesConditionToTrigger()
{
    const auto node = Node::create(R"(
        - conditions: $a
          gestures:
            - type: press
    )");
    const auto triggers = node->as<std::vector<std::unique_ptr<Trigger>>>();

    QCOMPARE(triggers.size(), 1);
    QVERIFY(dynamic_pointer_cast<VariableCondition>(triggers[0]->activationCondition()));
}

void TestTriggerGroupNodeParser::condition_triggerWithSingleCondition__mergesConditionsIntoAllGroup()
{
    const auto node = Node::create(R"(
        - conditions: $a
          gestures:
            - type: press
              conditions: $b
    )");
    const auto triggers = node->as<std::vector<std::unique_ptr<Trigger>>>();

    QCOMPARE(triggers.size(), 1);

    const auto condition = std::dynamic_pointer_cast<ConditionGroup>(triggers[0]->activationCondition());
    QVERIFY(condition);
    QCOMPARE(condition->negate(), false);
    QCOMPARE(condition->mode(), ConditionGroupMode::All);

    const auto &items = condition->conditions();
    QCOMPARE(items.size(), 2);

    const auto firstCondition = std::dynamic_pointer_cast<VariableCondition>(items[0]);
    QVERIFY(firstCondition);
    QCOMPARE(firstCondition->variableName(), "a");

    const auto secondCondition = std::dynamic_pointer_cast<VariableCondition>(items[1]);
    QVERIFY(secondCondition);
    QCOMPARE(secondCondition->variableName(), "b");
}

void TestTriggerGroupNodeParser::nested_condition_triggerWithSingleCondition__mergesConditionsIntoAllGroup()
{
    const auto node = Node::create(R"(
        - conditions: $a
          gestures:
            - conditions: $b
              gestures:
                - type: press
                  conditions: $c
    )");
    const auto triggers = node->as<std::vector<std::unique_ptr<Trigger>>>();

    QCOMPARE(triggers.size(), 1);

    const auto condition = std::dynamic_pointer_cast<ConditionGroup>(triggers[0]->activationCondition());
    QVERIFY(condition);
    QCOMPARE(condition->negate(), false);
    QCOMPARE(condition->mode(), ConditionGroupMode::All);

    const auto &items = condition->conditions();
    QCOMPARE(items.size(), 3);

    const auto firstCondition = std::dynamic_pointer_cast<VariableCondition>(items[0]);
    QVERIFY(firstCondition);
    QCOMPARE(firstCondition->variableName(), "a");

    const auto secondCondition = std::dynamic_pointer_cast<VariableCondition>(items[1]);
    QVERIFY(secondCondition);
    QCOMPARE(secondCondition->variableName(), "b");

    const auto thirdCondition = std::dynamic_pointer_cast<VariableCondition>(items[2]);
    QVERIFY(thirdCondition);
    QCOMPARE(thirdCondition->variableName(), "c");
}

void TestTriggerGroupNodeParser::condition_triggerWithAllConditionGroup__prependsConditionToTrigger()
{
    const auto node = Node::create(R"(
        - conditions: $a
          gestures:
            - type: press
              conditions:
                - $b
                - $c
    )");
    const auto triggers = node->as<std::vector<std::unique_ptr<Trigger>>>();

    QCOMPARE(triggers.size(), 1);

    const auto condition = std::dynamic_pointer_cast<ConditionGroup>(triggers[0]->activationCondition());
    QVERIFY(condition);

    const auto &items = condition->conditions();
    QCOMPARE(items.size(), 3);
    QCOMPARE(condition->negate(), false);
    QCOMPARE(condition->mode(), ConditionGroupMode::All);

    const auto firstCondition = std::dynamic_pointer_cast<VariableCondition>(items[0]);
    QVERIFY(firstCondition);
    QCOMPARE(firstCondition->variableName(), "a");

    const auto secondCondition = std::dynamic_pointer_cast<VariableCondition>(items[1]);
    QVERIFY(secondCondition);
    QCOMPARE(secondCondition->variableName(), "b");

    const auto thirdCondition = std::dynamic_pointer_cast<VariableCondition>(items[2]);
    QVERIFY(thirdCondition);
    QCOMPARE(thirdCondition->variableName(), "c");
}

void TestTriggerGroupNodeParser::condition_triggerWithAnyConditionGroup__mergesConditionsIntoAllGroup()
{
    const auto node = Node::create(R"(
        - conditions: $a
          gestures:
            - type: press
              conditions:
                any:
                  - $b
                  - $c
    )");
    const auto triggers = node->as<std::vector<std::unique_ptr<Trigger>>>();

    QCOMPARE(triggers.size(), 1);

    const auto condition = std::dynamic_pointer_cast<ConditionGroup>(triggers[0]->activationCondition());
    QVERIFY(condition);
    QCOMPARE(condition->negate(), false);
    QCOMPARE(condition->mode(), ConditionGroupMode::All);

    const auto &items = condition->conditions();
    QCOMPARE(items.size(), 2);

    const auto firstCondition = std::dynamic_pointer_cast<VariableCondition>(items[0]);
    QVERIFY(firstCondition);
    QCOMPARE(firstCondition->variableName(), "a");

    const auto secondCondition = std::dynamic_pointer_cast<ConditionGroup>(items[1]);
    QVERIFY(secondCondition);
    QCOMPARE(secondCondition->negate(), false);
    QCOMPARE(secondCondition->mode(), ConditionGroupMode::Any);

    const auto &secondConditionItems = secondCondition->conditions();
    QCOMPARE(secondConditionItems.size(), 2);

    const auto thirdCondition = std::dynamic_pointer_cast<VariableCondition>(secondConditionItems[0]);
    QVERIFY(thirdCondition);
    QCOMPARE(thirdCondition->variableName(), "b");

    const auto fourthCondition = std::dynamic_pointer_cast<VariableCondition>(secondConditionItems[1]);
    QVERIFY(fourthCondition);
    QCOMPARE(fourthCondition->variableName(), "c");
}

void TestTriggerGroupNodeParser::invalidPropertyValue__throwsExceptionAtCorrectPosition()
{
    const auto node = Node::create(R"(
        - instant: _
          gestures:
            - type: press
    )");

    INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<std::vector<std::unique_ptr<Trigger>>>(), InvalidValueConfigException, 1, 19);
}

void TestTriggerGroupNodeParser::unusedProperty__addsConfigIssueAtCorrectPosition()
{
    const auto node = Node::create(R"(
        - _: _
          gestures:
            - type: press
    )");

    node->as<std::vector<std::unique_ptr<Trigger>>>();
    INPUTACTIONS_VERIFY_ADDS_CONFIG_ISSUE_SAVE(node->addUnusedMapPropertyIssues(), UnusedPropertyConfigIssue, 1, 10, issue);
    QCOMPARE(issue->property(), "_");
}

}

QTEST_MAIN(InputActions::TestTriggerGroupNodeParser)