#include "TestConditionNodeParser.h"
#include "libinputactions/conditions/VariableCondition.h"
#include "libinputactions/config/ConfigIssue.h"
#include "libinputactions/config/ConfigIssueManager.h"
#include "libinputactions/config/parsers/core.h"
#include "libinputactions/globals.h"
#include "libinputactions/variables/VariableManager.h"
#include <libinputactions/conditions/ConditionGroup.h>
#include <libinputactions/config/Node.h>
#include <memory>

namespace InputActions
{

void TestConditionNodeParser::init()
{
    g_variableManager = std::make_shared<VariableManager>();
}

void TestConditionNodeParser::group_all__parsedCorrectly()
{
    const auto node = Node::load("all: []");
    const auto condition = std::dynamic_pointer_cast<ConditionGroup>(node.as<std::shared_ptr<Condition>>());

    QCOMPARE(condition->mode(), ConditionGroupMode::All);
}

void TestConditionNodeParser::group_any__parsedCorrectly()
{
    const auto node = Node::load("any: []");
    const auto condition = std::dynamic_pointer_cast<ConditionGroup>(node.as<std::shared_ptr<Condition>>());

    QCOMPARE(condition->mode(), ConditionGroupMode::Any);
}

void TestConditionNodeParser::group_none__parsedCorrectly()
{
    const auto node = Node::load("none: []");
    const auto condition = std::dynamic_pointer_cast<ConditionGroup>(node.as<std::shared_ptr<Condition>>());

    QCOMPARE(condition->mode(), ConditionGroupMode::None);
}

void TestConditionNodeParser::group_implicitAll__parsedAsAllGroup()
{
    const auto node = Node::load("[]");
    const auto condition = std::dynamic_pointer_cast<ConditionGroup>(node.as<std::shared_ptr<Condition>>());

    QCOMPARE(condition->mode(), ConditionGroupMode::All);
}

void TestConditionNodeParser::variable_boolVariableWithoutOperator__parsedCorrectly()
{
    const auto node = Node::load("$a");
    g_variableManager->registerLocalVariable<bool>("a");
    const auto condition = std::dynamic_pointer_cast<VariableCondition>(node.as<std::shared_ptr<Condition>>());

    Q_ASSERT(!condition->negate());
    QCOMPARE(condition->comparisonOperator(), ComparisonOperator::EqualTo);
    QCOMPARE(condition->variableName(), "a");
    QCOMPARE(condition->values().size(), 1);

    const auto value = condition->values()[0].get().value();
    Q_ASSERT(std::any_cast<bool>(value));
}

void TestConditionNodeParser::variable_negatedBoolVariableWithoutOperator__parsedCorrectly()
{
    const auto node = Node::load("_: !$a").at("_");
    g_variableManager->registerLocalVariable<bool>("a");
    const auto condition = std::dynamic_pointer_cast<VariableCondition>(node->as<std::shared_ptr<Condition>>());

    Q_ASSERT(condition->negate());
    QCOMPARE(condition->comparisonOperator(), ComparisonOperator::EqualTo);
    QCOMPARE(condition->variableName(), "a");
    QCOMPARE(condition->values().size(), 1);

    const auto value = condition->values()[0].get().value();
    Q_ASSERT(std::any_cast<bool>(value));
}

void TestConditionNodeParser::variable_matchesOperator_invalidRegex__throwsInvalidValueConfigException()
{
    const auto node = Node::load("$a matches (");
    g_variableManager->registerLocalVariable<QString>("a");

    QVERIFY_THROWS_EXCEPTION(InvalidValueConfigException, node.as<std::shared_ptr<Condition>>());
}

void TestConditionNodeParser::variable_invalidVariable__throwsInvalidVariableConfigException()
{
    const auto node = Node::load("$a");

    QVERIFY_THROWS_EXCEPTION(InvalidVariableConfigException, node.as<std::shared_ptr<Condition>>());
}

void TestConditionNodeParser::variable_inGroups__variableManagerPropagated_doesNotThrow()
{
    const auto node = Node::load(R"(
        all:
          - any:
              - none:
                  - $a
    )");

    VariableManager variableManager;
    variableManager.registerLocalVariable<bool>("a");

    QVERIFY_THROWS_NO_EXCEPTION(parseCondition(&node, &variableManager));
}

void TestConditionNodeParser::legacy_windowClass__parsedCorrectly()
{
    const auto node = Node::load("window_class: a");
    const auto condition = std::dynamic_pointer_cast<ConditionGroup>(node.as<std::shared_ptr<Condition>>());

    QCOMPARE(condition->mode(), ConditionGroupMode::Any);
    QCOMPARE(condition->negate(), false);

    const auto &conditionGroupItems = condition->conditions();

    const auto subCondition1 = std::dynamic_pointer_cast<VariableCondition>(conditionGroupItems[0]);
    QCOMPARE(subCondition1->variableName(), "window_class");
    QCOMPARE(subCondition1->comparisonOperator(), ComparisonOperator::Regex);
    QCOMPARE(std::any_cast<QString>(subCondition1->values()[0].get().value()), "a");

    const auto subCondition2 = std::dynamic_pointer_cast<VariableCondition>(conditionGroupItems[1]);
    QCOMPARE(subCondition2->variableName(), "window_name");
    QCOMPARE(subCondition2->comparisonOperator(), ComparisonOperator::Regex);
    QCOMPARE(std::any_cast<QString>(subCondition2->values()[0].get().value()), "a");
}

void TestConditionNodeParser::legacy_windowClass__addsDeprecatedFeatureIssue()
{
    const auto node = Node::load("window_class: a");
    node.as<std::shared_ptr<Condition>>();

    const auto *issue = g_configIssueManager->findIssueByType<DeprecatedFeatureConfigIssue>();
    QVERIFY(issue);
    QCOMPARE(issue->feature(), DeprecatedFeature::LegacyConditions);
}

void TestConditionNodeParser::legacy_windowClass_negated__parsedCorrectly()
{
    const auto node = Node::load(R"(
        window_class: a
        negate: [ window_class ]
    )");
    const auto condition = std::dynamic_pointer_cast<ConditionGroup>(node.as<std::shared_ptr<Condition>>());

    QCOMPARE(condition->negate(), true);
}

void TestConditionNodeParser::legacy_windowState__parsedCorrectly()
{
    const auto node = Node::load("window_state: [ fullscreen, maximized ]");
    const auto condition = std::dynamic_pointer_cast<ConditionGroup>(node.as<std::shared_ptr<Condition>>());

    QCOMPARE(condition->mode(), ConditionGroupMode::Any);
    QCOMPARE(condition->negate(), false);

    const auto &conditionGroupItems = condition->conditions();

    const auto subCondition1 = std::dynamic_pointer_cast<VariableCondition>(conditionGroupItems[0]);
    QCOMPARE(subCondition1->variableName(), "window_fullscreen");
    QCOMPARE(subCondition1->comparisonOperator(), ComparisonOperator::EqualTo);
    QVERIFY(std::any_cast<bool>(subCondition1->values()[0].get().value()));

    const auto subCondition2 = std::dynamic_pointer_cast<VariableCondition>(conditionGroupItems[1]);
    QCOMPARE(subCondition2->variableName(), "window_maximized");
    QCOMPARE(subCondition2->comparisonOperator(), ComparisonOperator::EqualTo);
    QVERIFY(std::any_cast<bool>(subCondition1->values()[0].get().value()));
}

void TestConditionNodeParser::legacy_windowState__addsDeprecatedFeatureIssue()
{
    const auto node = Node::load("window_state: [ fullscreen ]");
    node.as<std::shared_ptr<Condition>>();

    const auto *issue = g_configIssueManager->findIssueByType<DeprecatedFeatureConfigIssue>();
    QVERIFY(issue);
    QCOMPARE(issue->feature(), DeprecatedFeature::LegacyConditions);
}

void TestConditionNodeParser::legacy_windowState_negated__parsedCorrectly()
{
    const auto node = Node::load(R"(
        window_state: [ fullscreen, maximized ]
        negate: [ window_state ]
    )");
    const auto condition = std::dynamic_pointer_cast<ConditionGroup>(node.as<std::shared_ptr<Condition>>());

    QCOMPARE(condition->negate(), true);
}

void TestConditionNodeParser::legacy_and__parsedAsAllGroup()
{
    const auto node = Node::load(R"(
        window_class: kwrite
        window_state: [ maximized ]
    )");
    const auto condition = std::dynamic_pointer_cast<ConditionGroup>(node.as<std::shared_ptr<Condition>>());
    const auto &conditionGroupItems = condition->conditions();

    QCOMPARE(condition->mode(), ConditionGroupMode::All);
    QCOMPARE(conditionGroupItems.size(), 2);
}

void TestConditionNodeParser::legacy_or__parsedAsAnyGroup()
{
    const auto node = Node::load(R"(
        - window_class: kwrite
        - window_class: konsole
    )");
    const auto condition = std::dynamic_pointer_cast<ConditionGroup>(node.as<std::shared_ptr<Condition>>());

    QCOMPARE(condition->mode(), ConditionGroupMode::Any);
}

void TestConditionNodeParser::legacy_mixedWithNormal_normalFirst__throwsInvalidValueConfigException()
{
    const auto node = Node::load(R"(
        - $window_class == kwrite
        - window_class: kwrite
    )");

    QVERIFY_THROWS_EXCEPTION(InvalidValueConfigException, node.as<std::shared_ptr<Condition>>());
}

void TestConditionNodeParser::legacy_mixedWithNormal_legacyFirst__throwsInvalidValueConfigException()
{
    const auto node = Node::load(R"(
        - window_class: kwrite
        - $window_class == kwrite
    )");

    QVERIFY_THROWS_EXCEPTION(InvalidValueConfigException, node.as<std::shared_ptr<Condition>>());
}

}

QTEST_MAIN(InputActions::TestConditionNodeParser)
#include "TestConditionNodeParser.moc"