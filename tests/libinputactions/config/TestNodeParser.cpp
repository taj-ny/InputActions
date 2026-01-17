#include "TestNodeParser.h"
#include "libinputactions/conditions/LazyCondition.h"
#include "libinputactions/config/ConfigIssueManager.h"
#include "libinputactions/globals.h"
#include "libinputactions/variables/VariableManager.h"
#include <libinputactions/conditions/ConditionGroup.h>
#include <libinputactions/conditions/VariableCondition.h>
#include <libinputactions/config/Node.h>
#include <libinputactions/triggers/Trigger.h>
#include <memory>

namespace InputActions
{

void TestNodeParser::init()
{
    g_variableManager->registerLocalVariable<bool>("a");
    g_variableManager->registerLocalVariable<bool>("b");
    g_variableManager->registerLocalVariable<bool>("c");
    g_variableManager->registerLocalVariable<bool>("d");
    g_configIssueManager = std::make_shared<ConfigIssueManager>("");
}

void TestNodeParser::variableCondition_boolVariableWithoutOperator__constructedCorrectly()
{
    const auto node = load("$a");
    const auto condition = node.as<std::shared_ptr<Condition>>();

    const auto *variableCondition = lazyConditionToVariable(condition);
    Q_ASSERT(!variableCondition->negate());
    QCOMPARE(variableCondition->comparisonOperator(), ComparisonOperator::EqualTo);
    QCOMPARE(variableCondition->variableName(), "a");
    QCOMPARE(variableCondition->values().size(), 1);

    const auto value = variableCondition->values()[0].get().value();
    Q_ASSERT(std::any_cast<bool>(value));
}

void TestNodeParser::variableCondition_negatedBoolVariableWithoutOperator__constructedCorrectly()
{
    const auto node = load("_: !$a").at("_");
    const auto condition = node->as<std::shared_ptr<Condition>>();

    const auto *variableCondition = lazyConditionToVariable(condition);
    Q_ASSERT(variableCondition->negate());
    QCOMPARE(variableCondition->comparisonOperator(), ComparisonOperator::EqualTo);
    QCOMPARE(variableCondition->variableName(), "a");
    QCOMPARE(variableCondition->values().size(), 1);

    const auto value = variableCondition->values()[0].get().value();
    Q_ASSERT(std::any_cast<bool>(value));
}

void TestNodeParser::triggerGroup_propertyAppliedToTrigger()
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
    Q_ASSERT(lazyConditionToVariable(triggers[0]->activationCondition()));
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

    const auto conditionGroup = std::dynamic_pointer_cast<ConditionGroup>(triggers[0]->activationCondition());
    Q_ASSERT(conditionGroup);
    QCOMPARE(conditionGroup->mode(), ConditionGroupMode::All);

    const auto &conditionGroupItems = conditionGroup->conditions();
    QCOMPARE(conditionGroupItems.size(), 2);

    QCOMPARE(lazyConditionToVariable(conditionGroupItems[0])->variableName(), "a");
    QCOMPARE(lazyConditionToVariable(conditionGroupItems[1])->variableName(), "b");
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
    const auto condition = triggers[0]->activationCondition();

    const auto conditionGroup = std::dynamic_pointer_cast<ConditionGroup>(condition);
    const auto &conditionGroupItems = conditionGroup->conditions();

    QCOMPARE(conditionGroupItems.size(), 3);
    QCOMPARE(lazyConditionToVariable(conditionGroupItems[0])->variableName(), "a");
    QCOMPARE(lazyConditionToVariable(conditionGroupItems[1])->variableName(), "b");
    QCOMPARE(lazyConditionToVariable(conditionGroupItems[2])->variableName(), "c");
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
    QCOMPARE(lazyConditionToVariable(conditionGroupItems[0])->variableName(), "a");
    QCOMPARE(lazyConditionToVariable(conditionGroupItems[1])->variableName(), "b");
    QCOMPARE(lazyConditionToVariable(conditionGroupItems[2])->variableName(), "c");
}

void TestNodeParser::triggerGroup_condition_triggerWithAnyConditionGroup__()
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
    
    QCOMPARE(lazyConditionToVariable(conditionGroupItems[0])->variableName(), "a");

    const auto conditionGroup2 = std::dynamic_pointer_cast<ConditionGroup>(conditionGroupItems[1]);
    const auto &conditionGroup2Items = conditionGroup2->conditions();
    QCOMPARE(conditionGroup2->mode(), ConditionGroupMode::Any);

    QCOMPARE(lazyConditionToVariable(conditionGroup2Items[0])->variableName(), "b");
    QCOMPARE(lazyConditionToVariable(conditionGroup2Items[1])->variableName(), "c");
}

Node TestNodeParser::load(const QString &s)
{
    return Node(YAML::Load(s.toStdString()));
}

const VariableCondition *TestNodeParser::lazyConditionToVariable(const std::shared_ptr<Condition> &condition)
{
    const auto lazyCondition = std::dynamic_pointer_cast<LazyCondition>(condition);
    condition->evaluate();
    return dynamic_cast<const VariableCondition *>(lazyCondition->condition());
}

}

QTEST_MAIN(InputActions::TestNodeParser)
#include "TestNodeParser.moc"