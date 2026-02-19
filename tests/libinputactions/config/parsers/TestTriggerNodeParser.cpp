#include "TestTriggerNodeParser.h"
#include <libinputactions/conditions/ConditionGroup.h>
#include <libinputactions/conditions/VariableCondition.h>
#include <libinputactions/config/ConfigIssue.h>
#include <libinputactions/config/ConfigIssueManager.h>
#include <libinputactions/config/Node.h>
#include <libinputactions/triggers/Trigger.h>

namespace InputActions
{

void TestTriggerNodeParser::stroke_withConflictingBeginAction__throwsInvalidValueContextConfigException()
{
    const auto node = Node::create(R"(
        type: stroke
        strokes: [ 'MgAAMjJkZAA=' ]

        actions:
          - on: begin
            command: _
    )");

    INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<std::unique_ptr<Trigger>>(), InvalidValueContextConfigException, 5, 12);
}

void TestTriggerNodeParser::stroke_withNonConflictingBeginAction__doesNotThrow()
{
    const auto node = Node::create(R"(
        type: stroke
        strokes: [ 'MgAAMjJkZAA=' ]

        actions:
          - on: begin
            conflicting: false
            command: _
    )");

    QVERIFY_THROWS_NO_EXCEPTION(node->as<std::unique_ptr<Trigger>>());
}

void TestTriggerNodeParser::stroke_withEndAction__doesNotThrow()
{
    const auto node = Node::create(R"(
        type: stroke
        strokes: [ 'MgAAMjJkZAA=' ]

        actions:
          - on: end
            command: _
    )");

    QVERIFY_THROWS_NO_EXCEPTION(node->as<std::unique_ptr<Trigger>>());
}

void TestTriggerNodeParser::stroke_invalidStroke__throwsInvalidValueConfigException()
{
    const auto node = Node::create(R"(
        type: stroke
        strokes: [ 'MgAAMjJkZA=' ]

        actions:
          - on: end
            command: _
    )");

    INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<std::unique_ptr<Trigger>>(), InvalidValueConfigException, 2, 19);
}

void TestTriggerNodeParser::fingers__parsesNodeCorrectly()
{
    const auto node = Node::create(R"(
        type: press
        fingers: 2
    )");
    const auto trigger = node->as<std::unique_ptr<Trigger>>();

    const auto condition = std::dynamic_pointer_cast<VariableCondition>(trigger->activationCondition());
    QVERIFY(condition);
    QCOMPARE(condition->negate(), false);
    QCOMPARE(condition->variableName(), "fingers");
    QCOMPARE(condition->comparisonOperator(), ComparisonOperator::EqualTo);

    const auto &values = condition->values();
    QCOMPARE(values.size(), 1);
    QCOMPARE(std::any_cast<qreal>(values[0].get().value()), 2);
}

void TestTriggerNodeParser::fingers_range__parsesNodeCorrectly()
{
    const auto node = Node::create(R"(
        type: press
        fingers: 2-3
    )");
    const auto trigger = node->as<std::unique_ptr<Trigger>>();

    const auto condition = std::dynamic_pointer_cast<VariableCondition>(trigger->activationCondition());
    QVERIFY(condition);
    QCOMPARE(condition->negate(), false);
    QCOMPARE(condition->variableName(), "fingers");
    QCOMPARE(condition->comparisonOperator(), ComparisonOperator::Between);

    const auto &values = condition->values();
    QCOMPARE(values.size(), 2);
    QCOMPARE(std::any_cast<qreal>(values[0].get().value()), 2);
    QCOMPARE(std::any_cast<qreal>(values[1].get().value()), 3);
}

void TestTriggerNodeParser::mouseButtons_duplicateItem__throwsDuplicateSetItemConfigException()
{
    const auto node = Node::create(R"(
        type: press
        mouse_buttons: [ left, left ]
    )");

    INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION_SAVE(node->as<std::unique_ptr<Trigger>>(), DuplicateSetItemConfigException, 2, 31, e);
    QCOMPARE(e->index(), 1);
}

void TestTriggerNodeParser::keyboardModifiers__addsDeprecatedFeatureConfigIssue()
{
    const auto node = Node::create(R"(
        type: press
        keyboard_modifiers: none
    )");

    INPUTACTIONS_VERIFY_ADDS_CONFIG_ISSUE_SAVE(node->as<std::unique_ptr<Trigger>>(), DeprecatedFeatureConfigIssue, 2, 28, e);
    QCOMPARE(e->feature(), DeprecatedFeature::TriggerKeyboardModifiers);
}

void TestTriggerNodeParser::keyboardModifiers_any__doesNotAddCondition()
{
    const auto node = Node::create(R"(
        type: press
        keyboard_modifiers: any
    )");
    const auto trigger = node->as<std::unique_ptr<Trigger>>();

    QVERIFY(!trigger->activationCondition());
}

void TestTriggerNodeParser::keyboardModifiers_metaAlt__parsesNodeCorrectly()
{
    const auto node = Node::create(R"(
        type: press
        keyboard_modifiers: [ meta, alt ]
    )");
    const auto trigger = node->as<std::unique_ptr<Trigger>>();

    const auto condition = std::dynamic_pointer_cast<VariableCondition>(trigger->activationCondition());
    QVERIFY(condition);
    QCOMPARE(condition->negate(), false);
    QCOMPARE(condition->variableName(), "keyboard_modifiers");
    QCOMPARE(condition->comparisonOperator(), ComparisonOperator::EqualTo);

    const auto &values = condition->values();
    QCOMPARE(values.size(), 1);
    QCOMPARE(std::any_cast<Qt::KeyboardModifiers>(values[0].get().value()), Qt::KeyboardModifier::MetaModifier | Qt::KeyboardModifier::AltModifier);
}

void TestTriggerNodeParser::keyboardModifiers_none__parsesNodeCorrectly()
{
    const auto node = Node::create(R"(
        type: press
        keyboard_modifiers: none
    )");
    const auto trigger = node->as<std::unique_ptr<Trigger>>();

    const auto condition = std::dynamic_pointer_cast<VariableCondition>(trigger->activationCondition());
    QVERIFY(condition);
    QCOMPARE(condition->negate(), false);
    QCOMPARE(condition->variableName(), "keyboard_modifiers");
    QCOMPARE(condition->comparisonOperator(), ComparisonOperator::EqualTo);

    const auto &values = condition->values();
    QCOMPARE(values.size(), 1);
    QCOMPARE(std::any_cast<Qt::KeyboardModifiers>(values[0].get().value()), Qt::KeyboardModifier::NoModifier);
}

void TestTriggerNodeParser::keyboardModifiers_invalid__throwsInvalidValueConfigException()
{
    const auto node = Node::create(R"(
        type: press
        keyboard_modifiers: e
    )");

    INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<std::unique_ptr<Trigger>>(), InvalidValueConfigException, 2, 28);
}

void TestTriggerNodeParser::fingers_keyboardModifiers_triggerCondition__mergedIntoAllGroup()
{
    const auto node = Node::create(R"(
        type: press
        fingers: 2-3
        conditions: $window_maximized
        keyboard_modifiers: none
    )");
    const auto trigger = node->as<std::unique_ptr<Trigger>>();

    const auto condition = std::dynamic_pointer_cast<ConditionGroup>(trigger->activationCondition());
    QVERIFY(condition);
    QCOMPARE(condition->negate(), false);
    QCOMPARE(condition->mode(), ConditionGroupMode::All);
    QCOMPARE(condition->conditions().size(), 3);

    const auto fingersCondition = std::dynamic_pointer_cast<VariableCondition>(condition->conditions()[0]);
    QVERIFY(fingersCondition);
    QCOMPARE(fingersCondition->negate(), false);
    QCOMPARE(fingersCondition->variableName(), "fingers");

    const auto keyboardModifiersCondition = std::dynamic_pointer_cast<VariableCondition>(condition->conditions()[1]);
    QVERIFY(keyboardModifiersCondition);
    QCOMPARE(keyboardModifiersCondition->negate(), false);
    QCOMPARE(keyboardModifiersCondition->variableName(), "keyboard_modifiers");

    const auto windowMaximizedCondition = std::dynamic_pointer_cast<VariableCondition>(condition->conditions()[2]);
    QVERIFY(windowMaximizedCondition);
    QCOMPARE(windowMaximizedCondition->negate(), false);
    QCOMPARE(windowMaximizedCondition->variableName(), "window_maximized");
}

void TestTriggerNodeParser::invalid_noType__throwsMissingRequiredPropertyConfigException()
{
    const auto node = Node::create("_: _");

    INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION_SAVE(node->as<std::unique_ptr<Trigger>>(), MissingRequiredPropertyConfigException, 0, 0, e);
    QCOMPARE(e->property(), "type");
}

void TestTriggerNodeParser::invalid_invalidType__throwsInvalidValueConfigException()
{
    const auto node = Node::create("type: _");
    INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<std::unique_ptr<Trigger>>(), InvalidValueConfigException, 0, 6);
}

}

QTEST_MAIN(InputActions::TestTriggerNodeParser)