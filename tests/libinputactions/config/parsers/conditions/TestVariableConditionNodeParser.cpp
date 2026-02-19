#include "TestVariableConditionNodeParser.h"
#include <libinputactions/conditions/VariableCondition.h>
#include <libinputactions/config/ConfigIssue.h>
#include <libinputactions/config/Node.h>
#include <libinputactions/config/parsers/core.h>
#include <libinputactions/variables/VariableManager.h>

namespace InputActions
{

void TestVariableConditionNodeParser::init()
{
    g_variableManager = std::make_shared<VariableManager>();
    g_variableManager->registerLocalVariable<bool>("bool");
    g_variableManager->registerLocalVariable<qreal>("number");
    g_variableManager->registerLocalVariable<QPointF>("point");
    g_variableManager->registerLocalVariable<QString>("string");
    g_variableManager->registerLocalVariable<Qt::KeyboardModifiers>("keyboard_modifiers");
}

void TestVariableConditionNodeParser::boolVariableWithoutOperator__parsesNodeCorrectly()
{
    const auto node = Node::create("$bool");
    const auto condition = std::dynamic_pointer_cast<VariableCondition>(node->as<std::shared_ptr<Condition>>());

    QVERIFY(condition);
    QCOMPARE(condition->negate(), false);
    QCOMPARE(condition->variableName(), "bool");
    QCOMPARE(condition->comparisonOperator(), ComparisonOperator::EqualTo);

    const auto &values = condition->values();
    QCOMPARE(values.size(), 1);
    QCOMPARE(std::any_cast<bool>(values[0].get().value()), true);
}

void TestVariableConditionNodeParser::negatedBoolVariableWithoutOperator__parsesNodeCorrectly()
{
    const auto node = Node::create("_: !$bool")->at("_")->shared_from_this();
    const auto condition = std::dynamic_pointer_cast<VariableCondition>(node->as<std::shared_ptr<Condition>>());

    QVERIFY(condition);
    QCOMPARE(condition->negate(), true);
    QCOMPARE(condition->variableName(), "bool");
    QCOMPARE(condition->comparisonOperator(), ComparisonOperator::EqualTo);

    const auto &values = condition->values();
    QCOMPARE(values.size(), 1);
    QCOMPARE(std::any_cast<bool>(values[0].get().value()), true);
}

void TestVariableConditionNodeParser::negated__parsesNodeCorrectly()
{
    const auto node = Node::create("!$number == 1");
    const auto condition = std::dynamic_pointer_cast<VariableCondition>(node->as<std::shared_ptr<Condition>>());

    QVERIFY(condition);
    QCOMPARE(condition->negate(), true);
    QCOMPARE(condition->variableName(), "number");
    QCOMPARE(condition->comparisonOperator(), ComparisonOperator::EqualTo);

    const auto &values = condition->values();
    QCOMPARE(values.size(), 1);
    QCOMPARE(std::any_cast<qreal>(values[0].get().value()), 1);
}

void TestVariableConditionNodeParser::between__parsesNodeCorrectly()
{
    const auto node = Node::create("$number between 1;2");
    const auto condition = std::dynamic_pointer_cast<VariableCondition>(node->as<std::shared_ptr<Condition>>());

    QVERIFY(condition);
    QCOMPARE(condition->negate(), false);
    QCOMPARE(condition->variableName(), "number");
    QCOMPARE(condition->comparisonOperator(), ComparisonOperator::Between);

    const auto &values = condition->values();
    QCOMPARE(values.size(), 2);
    QCOMPARE(std::any_cast<qreal>(values[0].get().value()), 1);
    QCOMPARE(std::any_cast<qreal>(values[1].get().value()), 2);
}

void TestVariableConditionNodeParser::between_point__parsesNodeCorrectly()
{
    const auto node = Node::create("$point between 0.1,0.2;0.3,0.4");
    const auto condition = std::dynamic_pointer_cast<VariableCondition>(node->as<std::shared_ptr<Condition>>());

    QVERIFY(condition);
    QCOMPARE(condition->negate(), false);
    QCOMPARE(condition->variableName(), "point");
    QCOMPARE(condition->comparisonOperator(), ComparisonOperator::Between);

    const auto &values = condition->values();
    QCOMPARE(values.size(), 2);
    QCOMPARE(std::any_cast<QPointF>(values[0].get().value()), QPointF(0.1, 0.2));
    QCOMPARE(std::any_cast<QPointF>(values[1].get().value()), QPointF(0.3, 0.4));
}

void TestVariableConditionNodeParser::between_invalid_oneValue__throwsInvalidValueConfigException()
{
    const auto node = Node::create("$number between 1");
    INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<std::shared_ptr<Condition>>(), InvalidValueConfigException, 0, 16);
}

void TestVariableConditionNodeParser::between_invalid_threeValues__throwsInvalidValueConfigException()
{
    const auto node = Node::create("$number between 1;2;3");
    INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<std::shared_ptr<Condition>>(), InvalidValueConfigException, 0, 16);
}

void TestVariableConditionNodeParser::contains_string__parsesNodeCorrectly()
{
    const auto node = Node::create("$string contains a");
    const auto condition = std::dynamic_pointer_cast<VariableCondition>(node->as<std::shared_ptr<Condition>>());

    QVERIFY(condition);
    QCOMPARE(condition->negate(), false);
    QCOMPARE(condition->variableName(), "string");
    QCOMPARE(condition->comparisonOperator(), ComparisonOperator::Contains);

    const auto &values = condition->values();
    QCOMPARE(values.size(), 1);
    QCOMPARE(std::any_cast<QString>(values[0].get().value()), "a");
}

void TestVariableConditionNodeParser::contains_flags_sequence__parsesNodeCorrectly()
{
    const auto node = Node::create("$keyboard_modifiers contains [ ctrl, meta ]");
    const auto condition = std::dynamic_pointer_cast<VariableCondition>(node->as<std::shared_ptr<Condition>>());

    QVERIFY(condition);
    QCOMPARE(condition->negate(), false);
    QCOMPARE(condition->variableName(), "keyboard_modifiers");
    QCOMPARE(condition->comparisonOperator(), ComparisonOperator::Contains);

    const auto &values = condition->values();
    QCOMPARE(values.size(), 1);
    QCOMPARE(std::any_cast<Qt::KeyboardModifiers>(values[0].get().value()), Qt::KeyboardModifier::ControlModifier | Qt::KeyboardModifier::MetaModifier);
}

void TestVariableConditionNodeParser::contains_flags_scalar__parsesNodeCorrectly()
{
    const auto node = Node::create("$keyboard_modifiers contains meta");
    const auto condition = std::dynamic_pointer_cast<VariableCondition>(node->as<std::shared_ptr<Condition>>());

    QVERIFY(condition);
    QCOMPARE(condition->negate(), false);
    QCOMPARE(condition->variableName(), "keyboard_modifiers");
    QCOMPARE(condition->comparisonOperator(), ComparisonOperator::Contains);

    const auto &values = condition->values();
    QCOMPARE(values.size(), 1);
    QCOMPARE(std::any_cast<Qt::KeyboardModifiers>(values[0].get().value()), Qt::KeyboardModifier::MetaModifier);
}

void TestVariableConditionNodeParser::oneOf_sequence__parsesNodeCorrectly()
{
    const auto node = Node::create("$string one_of [ a, b ]");
    const auto condition = std::dynamic_pointer_cast<VariableCondition>(node->as<std::shared_ptr<Condition>>());

    QVERIFY(condition);
    QCOMPARE(condition->negate(), false);
    QCOMPARE(condition->variableName(), "string");
    QCOMPARE(condition->comparisonOperator(), ComparisonOperator::OneOf);

    const auto &values = condition->values();
    QCOMPARE(values.size(), 2);
    QCOMPARE(std::any_cast<QString>(values[0].get().value()), "a");
    QCOMPARE(std::any_cast<QString>(values[1].get().value()), "b");
}

void TestVariableConditionNodeParser::oneOf_scalar__parsesNodeCorrectly()
{
    const auto node = Node::create("$string one_of a");
    const auto condition = std::dynamic_pointer_cast<VariableCondition>(node->as<std::shared_ptr<Condition>>());

    QVERIFY(condition);
    QCOMPARE(condition->negate(), false);
    QCOMPARE(condition->variableName(), "string");
    QCOMPARE(condition->comparisonOperator(), ComparisonOperator::OneOf);

    const auto &values = condition->values();
    QCOMPARE(values.size(), 1);
    QCOMPARE(std::any_cast<QString>(values[0].get().value()), "a");
}

void TestVariableConditionNodeParser::matches__parsesNodeCorrectly()
{
    const auto node = Node::create("$string matches \"[a]\"");
    const auto condition = std::dynamic_pointer_cast<VariableCondition>(node->as<std::shared_ptr<Condition>>());

    QVERIFY(condition);
    QCOMPARE(condition->negate(), false);
    QCOMPARE(condition->variableName(), "string");
    QCOMPARE(condition->comparisonOperator(), ComparisonOperator::Regex);

    const auto &values = condition->values();
    QCOMPARE(values.size(), 1);
    QCOMPARE(std::any_cast<QString>(values[0].get().value()), "[a]");
}

void TestVariableConditionNodeParser::matches_invalidRegex__throwsInvalidValueConfigException()
{
    const auto node = Node::create("$string matches (");
    INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<std::shared_ptr<Condition>>(), InvalidValueConfigException, 0, 16);
}

void TestVariableConditionNodeParser::simpleOperators__parsesNodeCorrectly_data()
{
    QTest::addColumn<ComparisonOperator>("comparisonOperator");
    QTest::addColumn<QString>("raw");

    QTest::addRow("==") << ComparisonOperator::EqualTo << "==";
    QTest::addRow("!=") << ComparisonOperator::NotEqualTo << "!=";
    QTest::addRow(">") << ComparisonOperator::GreaterThan << ">";
    QTest::addRow(">=") << ComparisonOperator::GreaterThanOrEqual << ">=";
    QTest::addRow("<") << ComparisonOperator::LessThan << "<";
    QTest::addRow("<=") << ComparisonOperator::LessThanOrEqual << "<=";
}

void TestVariableConditionNodeParser::simpleOperators__parsesNodeCorrectly()
{
    QFETCH(ComparisonOperator, comparisonOperator);
    QFETCH(QString, raw);

    const auto node = Node::create(QString("$number %1 1").arg(raw));
    const auto condition = std::dynamic_pointer_cast<VariableCondition>(node->as<std::shared_ptr<Condition>>());

    QVERIFY(condition);
    QCOMPARE(condition->negate(), false);
    QCOMPARE(condition->variableName(), "number");
    QCOMPARE(condition->comparisonOperator(), comparisonOperator);

    const auto &values = condition->values();
    QCOMPARE(values.size(), 1);
    QCOMPARE(std::any_cast<qreal>(values[0].get().value()), 1);
}

void TestVariableConditionNodeParser::inGroups__variableManagerPropagated_doesNotThrow()
{
    const auto node = Node::create(R"(
        all:
          - any:
              - none:
                  - $b
    )");

    VariableManager variableManager;
    variableManager.registerLocalVariable<bool>("b");

    QVERIFY_THROWS_NO_EXCEPTION(parseCondition(node.get(), &variableManager));
}

void TestVariableConditionNodeParser::invalid_invalidVariable__throwsInvalidValueConfigException()
{
    const auto node = Node::create("$_");

    INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION_SAVE(node->as<std::shared_ptr<Condition>>(), InvalidVariableConfigException, 0, 0, e);
    QCOMPARE(e->variableName(), "_");
}

void TestVariableConditionNodeParser::invalid_noOperator__throwsInvalidValueConfigException()
{
    const auto node = Node::create("$number");
    INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<std::shared_ptr<Condition>>(), InvalidValueConfigException, 0, 1);
}

void TestVariableConditionNodeParser::invalid_noValue__throwsInvalidValueConfigException()
{
    const auto node = Node::create("$number ==");
    INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<std::shared_ptr<Condition>>(), InvalidValueConfigException, 0, 0);
}

}

QTEST_MAIN(InputActions::TestVariableConditionNodeParser)