#include "TestDeviceRuleNodeParser.h"
#include <libinputactions/conditions/VariableCondition.h>
#include <libinputactions/config/ConfigIssueManager.h>
#include <libinputactions/config/Node.h>
#include <libinputactions/config/parsers/containers.h>
#include <libinputactions/input/devices/InputDeviceRule.h>

namespace InputActions
{

void TestDeviceRuleNodeParser::valid__parsesNodeCorrectly()
{
    const auto node = Node::create(R"(
        device_rules:
          - conditions: $mouse
            grab: true

          - conditions: $touchpad
            ignore: true
    )");

    const auto rules = node->as<std::vector<InputDeviceRule>>();
    QCOMPARE(rules.size(), 2);

    const auto &mouseRule = rules[0];
    QVERIFY(mouseRule.condition());
    QCOMPARE(mouseRule.properties().m_grab, true);
    QCOMPARE(mouseRule.properties().m_ignore, std::nullopt);

    const auto &touchpadRule = rules[1];
    QVERIFY(touchpadRule.condition());
    QCOMPARE(touchpadRule.properties().m_grab, std::nullopt);
    QCOMPARE(touchpadRule.properties().m_ignore, true);
}

void TestDeviceRuleNodeParser::mouse_triggerHandlerSetting_motionTimeout__parsesNodeCorrectly()
{
    const auto node = Node::create(R"(
        mouse:
          motion_timeout: 3735928559
    )");

    const auto rules = node->as<std::vector<InputDeviceRule>>();
    QCOMPARE(rules.size(), 1);

    const auto &rule = rules[0];
    QCOMPARE(rule.properties().m_mouseMotionTimeout, std::chrono::milliseconds(3735928559));

    const auto condition = std::dynamic_pointer_cast<VariableCondition>(rule.condition());
    QVERIFY(condition);
    QCOMPARE(condition->variableName(), "mouse");
    QCOMPARE(condition->comparisonOperator(), ComparisonOperator::EqualTo);

    const auto &conditionValues = condition->values();
    QCOMPARE(conditionValues.size(), 1);
    QCOMPARE(std::any_cast<bool>(conditionValues[0].get().value()), true);
}

void TestDeviceRuleNodeParser::mouse_triggerHandlerSetting_motionTimeout__addsDeprecatedFeatureConfigIssue()
{
    const auto node = Node::create(R"(
        mouse:
          motion_timeout: 1
    )");

    INPUTACTIONS_VERIFY_ADDS_CONFIG_ISSUE_SAVE(node->as<std::vector<InputDeviceRule>>(), DeprecatedFeatureConfigIssue, 2, 26, issue);
    QCOMPARE(issue->feature(), DeprecatedFeature::TriggerHandlerSettings);
}

void TestDeviceRuleNodeParser::mouse_triggerHandlerSetting_pressTimeout__parsesNodeCorrectly()
{
    const auto node = Node::create(R"(
        mouse:
          press_timeout: 3735928559
    )");

    const auto rules = node->as<std::vector<InputDeviceRule>>();
    QCOMPARE(rules.size(), 1);

    const auto &rule = rules[0];
    QCOMPARE(rule.properties().m_mousePressTimeout, std::chrono::milliseconds(3735928559));

    const auto condition = std::dynamic_pointer_cast<VariableCondition>(rule.condition());
    QVERIFY(condition);
    QCOMPARE(condition->variableName(), "mouse");
    QCOMPARE(condition->comparisonOperator(), ComparisonOperator::EqualTo);

    const auto &conditionValues = condition->values();
    QCOMPARE(conditionValues.size(), 1);
    QCOMPARE(std::any_cast<bool>(conditionValues[0].get().value()), true);
}

void TestDeviceRuleNodeParser::mouse_triggerHandlerSetting_pressTimeout__addsDeprecatedFeatureConfigIssue()
{
    const auto node = Node::create(R"(
        mouse:
          press_timeout: 1
    )");

    INPUTACTIONS_VERIFY_ADDS_CONFIG_ISSUE_SAVE(node->as<std::vector<InputDeviceRule>>(), DeprecatedFeatureConfigIssue, 2, 25, issue);
    QCOMPARE(issue->feature(), DeprecatedFeature::TriggerHandlerSettings);
}

void TestDeviceRuleNodeParser::mouse_triggerHandlerSetting_unblockButtonsOnTimeout__parsesNodeCorrectly()
{
    const auto node = Node::create(R"(
        mouse:
          unblock_buttons_on_timeout: true
    )");

    const auto rules = node->as<std::vector<InputDeviceRule>>();
    QCOMPARE(rules.size(), 1);

    const auto &rule = rules[0];
    QCOMPARE(rule.properties().m_mouseUnblockButtonsOnTimeout, true);

    const auto condition = std::dynamic_pointer_cast<VariableCondition>(rule.condition());
    QVERIFY(condition);
    QCOMPARE(condition->variableName(), "mouse");
    QCOMPARE(condition->comparisonOperator(), ComparisonOperator::EqualTo);

    const auto &conditionValues = condition->values();
    QCOMPARE(conditionValues.size(), 1);
    QCOMPARE(std::any_cast<bool>(conditionValues[0].get().value()), true);
}

void TestDeviceRuleNodeParser::mouse_triggerHandlerSetting_unblockButtonsOnTimeout__addsDeprecatedFeatureConfigIssue()
{
    const auto node = Node::create(R"(
        mouse:
          unblock_buttons_on_timeout: true
    )");

    INPUTACTIONS_VERIFY_ADDS_CONFIG_ISSUE_SAVE(node->as<std::vector<InputDeviceRule>>(), DeprecatedFeatureConfigIssue, 2, 38, issue);
    QCOMPARE(issue->feature(), DeprecatedFeature::TriggerHandlerSettings);
}

void TestDeviceRuleNodeParser::touchpad_devicesNode__parsesNodeCorrectly()
{
    const auto node = Node::create(R"(
        touchpad:
          devices:
            a:
              ignore: true
    )");

    const auto rules = node->as<std::vector<InputDeviceRule>>();
    QCOMPARE(rules.size(), 1);

    const auto &rule = rules[0];
    QCOMPARE(rule.properties().m_ignore, true);

    const auto condition = std::dynamic_pointer_cast<VariableCondition>(rule.condition());
    QVERIFY(condition);
    QCOMPARE(condition->variableName(), "name");
    QCOMPARE(condition->comparisonOperator(), ComparisonOperator::EqualTo);

    const auto &conditionValues = condition->values();
    QCOMPARE(conditionValues.size(), 1);
    QCOMPARE(std::any_cast<QString>(conditionValues[0].get().value()), "a");
}

void TestDeviceRuleNodeParser::touchpad_devicesNode__addsDeprecatedFeatureConfigIssue()
{
    const auto node = Node::create(R"(
        touchpad:
          devices:
            a:
              ignore: true
    )");

    INPUTACTIONS_VERIFY_ADDS_CONFIG_ISSUE_SAVE(node->as<std::vector<InputDeviceRule>>(), DeprecatedFeatureConfigIssue, 3, 12, issue);
    QCOMPARE(issue->feature(), DeprecatedFeature::TouchpadDevicesNode);
}

void TestDeviceRuleNodeParser::touchpad_triggerHandlerSetting_clickTimeout__parsesNodeCorrectly()
{
    const auto node = Node::create(R"(
        touchpad:
          click_timeout: 3735928559
    )");

    const auto rules = node->as<std::vector<InputDeviceRule>>();
    QCOMPARE(rules.size(), 1);

    const auto &rule = rules[0];
    QCOMPARE(rule.properties().m_touchpadClickTimeout, std::chrono::milliseconds(3735928559));

    const auto condition = std::dynamic_pointer_cast<VariableCondition>(rule.condition());
    QVERIFY(condition);
    QCOMPARE(condition->variableName(), "touchpad");
    QCOMPARE(condition->comparisonOperator(), ComparisonOperator::EqualTo);

    const auto &conditionValues = condition->values();
    QCOMPARE(conditionValues.size(), 1);
    QCOMPARE(std::any_cast<bool>(conditionValues[0].get().value()), true);
}

void TestDeviceRuleNodeParser::touchpad_triggerHandlerSetting_clickTimeout__addsDeprecatedFeatureConfigIssue()
{
    const auto node = Node::create(R"(
        touchpad:
          click_timeout: 1
    )");

    INPUTACTIONS_VERIFY_ADDS_CONFIG_ISSUE_SAVE(node->as<std::vector<InputDeviceRule>>(), DeprecatedFeatureConfigIssue, 2, 25, issue);
    QCOMPARE(issue->feature(), DeprecatedFeature::TriggerHandlerSettings);
}

}

QTEST_MAIN(InputActions::TestDeviceRuleNodeParser)