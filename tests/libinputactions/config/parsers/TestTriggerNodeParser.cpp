#include "TestTriggerNodeParser.h"
#include "libinputactions/conditions/ConditionGroup.h"
#include "libinputactions/conditions/VariableCondition.h"
#include "libinputactions/globals.h"
#include <libinputactions/config/Node.h>
#include <libinputactions/config/ConfigIssueManager.h>
#include <libinputactions/config/parsers/flags.h>
#include <libinputactions/triggers/Trigger.h>
#include <memory>

namespace InputActions
{

void TestTriggerNodeParser::noType__throwsConfigParserException()
{
    const auto node = Node::load("_: _");
    QVERIFY_THROWS_EXCEPTION(ConfigParserException, node.as<std::unique_ptr<Trigger>>());
}

void TestTriggerNodeParser::invalidType__throwsConfigParserException()
{
    const auto node = Node::load("type: _");
    QVERIFY_THROWS_EXCEPTION(ConfigParserException, node.as<std::unique_ptr<Trigger>>());
}

void TestTriggerNodeParser::stroke_withBeginAction__throwsConfigParserException()
{
    const auto node = Node::load(R"(
        type: stroke
        strokes: [ 'MgAAMjJkZAA=' ]

        actions:
          - on: begin
            command: _
    )");
    QVERIFY_THROWS_EXCEPTION(ConfigParserException, node.as<std::unique_ptr<Trigger>>());
}

void TestTriggerNodeParser::stroke_withEndAction__doesNotThrowConfigParserException()
{
    const auto node = Node::load(R"(
        type: stroke
        strokes: [ 'MgAAMjJkZAA=' ]

        actions:
          - on: end
            command: _
    )");
    QVERIFY_THROWS_NO_EXCEPTION(node.as<std::unique_ptr<Trigger>>());
}

void TestTriggerNodeParser::fingers__conditionCorrectlyConstructed()
{
    const auto node = Node::load(R"(
        type: press
        fingers: 2
    )");
    const auto trigger = node.as<std::unique_ptr<Trigger>>();

    const auto condition = std::dynamic_pointer_cast<VariableCondition>(trigger->activationCondition());
    QCOMPARE(condition->variableName(), "fingers");
    QCOMPARE(condition->comparisonOperator(), ComparisonOperator::EqualTo);
    QCOMPARE(std::any_cast<qreal>(condition->values()[0].get().value()), 2);
}

void TestTriggerNodeParser::fingers_range__conditionCorrectlyConstructed()
{
    const auto node = Node::load(R"(
        type: press
        fingers: 2-3
    )");
    const auto trigger = node.as<std::unique_ptr<Trigger>>();

    const auto condition = std::dynamic_pointer_cast<VariableCondition>(trigger->activationCondition());
    QCOMPARE(condition->variableName(), "fingers");
    QCOMPARE(condition->comparisonOperator(), ComparisonOperator::Between);
    QCOMPARE(std::any_cast<qreal>(condition->values()[0].get().value()), 2);
    QCOMPARE(std::any_cast<qreal>(condition->values()[1].get().value()), 3);
}

void TestTriggerNodeParser::fingers_triggerCondition__mergedIntoAllGroup()
{
    const auto node = Node::load(R"(
        type: press
        fingers: 2-3
        conditions: $window_maximized
    )");
    const auto trigger = node.as<std::unique_ptr<Trigger>>();

    const auto condition = std::dynamic_pointer_cast<ConditionGroup>(trigger->activationCondition());
    QCOMPARE(condition->mode(), ConditionGroupMode::All);

    const auto fingersCondition = std::dynamic_pointer_cast<VariableCondition>(condition->conditions()[0]);
    QCOMPARE(fingersCondition->variableName(), "fingers");
}

}

QTEST_MAIN(InputActions::TestTriggerNodeParser)
#include "TestTriggerNodeParser.moc"