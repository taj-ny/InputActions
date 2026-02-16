#include "TestInputActionNodeParser.h"
#include <libinputactions/actions/InputAction.h>
#include <libinputactions/config/ConfigIssue.h>
#include <libinputactions/config/Node.h>
#include <linux/input-event-codes.h>

namespace InputActions
{

void TestInputActionNodeParser::keyboardKey_twoSeparate__parsesNodeCorrectly()
{
    const auto node = Node::create("- keyboard: [ a, b ]");
    const auto items = node->as<std::vector<InputActionItem>>();

    QCOMPARE(items.size(), 4);
    QCOMPARE(items[0].keyboardPress, KEY_A);
    QCOMPARE(items[1].keyboardRelease, KEY_A);
    QCOMPARE(items[2].keyboardPress, KEY_B);
    QCOMPARE(items[3].keyboardRelease, KEY_B);
}

void TestInputActionNodeParser::keyboardKey_twoCombined__parsesNodeCorrectly()
{
    const auto node = Node::create("- keyboard: [ a+b ]");
    const auto items = node->as<std::vector<InputActionItem>>();

    QCOMPARE(items.size(), 4);
    QCOMPARE(items[0].keyboardPress, KEY_A);
    QCOMPARE(items[1].keyboardPress, KEY_B);
    QCOMPARE(items[2].keyboardRelease, KEY_B);
    QCOMPARE(items[3].keyboardRelease, KEY_A);
}

void TestInputActionNodeParser::keyboardKeyPress_two__parsesNodeCorrectly()
{
    const auto node = Node::create("- keyboard: [ +a, +b ]");
    const auto items = node->as<std::vector<InputActionItem>>();

    QCOMPARE(items.size(), 2);
    QCOMPARE(items[0].keyboardPress, KEY_A);
    QCOMPARE(items[1].keyboardPress, KEY_B);
}

void TestInputActionNodeParser::keyboardKeyRelease_two__parsesNodeCorrectly()
{
    const auto node = Node::create("- keyboard: [ -a, -b ]");
    const auto items = node->as<std::vector<InputActionItem>>();

    QCOMPARE(items.size(), 2);
    QCOMPARE(items[0].keyboardRelease, KEY_A);
    QCOMPARE(items[1].keyboardRelease, KEY_B);
}

void TestInputActionNodeParser::keyboardText__parsesNodeCorrectly()
{
    const auto node = Node::create("- keyboard: [ text: aaa ]");
    const auto items = node->as<std::vector<InputActionItem>>();

    QCOMPARE(items.size(), 1);
    QCOMPARE(items[0].keyboardText.get(), "aaa");
}

void TestInputActionNodeParser::keyboardText_command__parsesNodeCorrectly()
{
    const auto node = Node::create("- keyboard: [ text: { command: echo a } ]");
    const auto items = node->as<std::vector<InputActionItem>>();

    QCOMPARE(items.size(), 1);
    QCOMPARE(items[0].keyboardText.get(), "a\n");
}

void TestInputActionNodeParser::mouseButton_twoSeparate__parsesNodeCorrectly()
{
    const auto node = Node::create("- mouse: [ left, right ]");
    const auto items = node->as<std::vector<InputActionItem>>();

    QCOMPARE(items.size(), 4);
    QCOMPARE(items[0].mousePress, BTN_LEFT);
    QCOMPARE(items[1].mouseRelease, BTN_LEFT);
    QCOMPARE(items[2].mousePress, BTN_RIGHT);
    QCOMPARE(items[3].mouseRelease, BTN_RIGHT);
}

void TestInputActionNodeParser::mouseButton_twoCombined__parsesNodeCorrectly()
{
    const auto node = Node::create("- mouse: [ left+right ]");
    const auto items = node->as<std::vector<InputActionItem>>();

    QCOMPARE(items.size(), 4);
    QCOMPARE(items[0].mousePress, BTN_LEFT);
    QCOMPARE(items[1].mousePress, BTN_RIGHT);
    QCOMPARE(items[2].mouseRelease, BTN_RIGHT);
    QCOMPARE(items[3].mouseRelease, BTN_LEFT);
}

void TestInputActionNodeParser::mouseButtonPress_two__parsesNodeCorrectly()
{
    const auto node = Node::create("- mouse: [ +left, +right ]");
    const auto items = node->as<std::vector<InputActionItem>>();

    QCOMPARE(items.size(), 2);
    QCOMPARE(items[0].mousePress, BTN_LEFT);
    QCOMPARE(items[1].mousePress, BTN_RIGHT);
}

void TestInputActionNodeParser::mouseButtonRelease_two__parsesNodeCorrectly()
{
    const auto node = Node::create("- mouse: [ -left, -right ]");
    const auto items = node->as<std::vector<InputActionItem>>();

    QCOMPARE(items.size(), 2);
    QCOMPARE(items[0].mouseRelease, BTN_LEFT);
    QCOMPARE(items[1].mouseRelease, BTN_RIGHT);
}

void TestInputActionNodeParser::mouseMoveAbsolute__parsesNodeCorrectly()
{
    const auto node = Node::create("- mouse: [ move_to -1.2 1.2 ]");
    const auto items = node->as<std::vector<InputActionItem>>();

    QCOMPARE(items.size(), 1);
    QCOMPARE(items[0].mouseMoveAbsolute, QPointF(-1.2, 1.2));
}

void TestInputActionNodeParser::mouseMoveByDelta_noMultiplier__parsesNodeCorrectly()
{
    const auto node = Node::create("- mouse: [ move_by_delta ]");
    const auto items = node->as<std::vector<InputActionItem>>();

    QCOMPARE(items.size(), 1);
    QCOMPARE(items[0].mouseMoveRelativeByDelta, static_cast<qreal>(1));
}

void TestInputActionNodeParser::mouseMoveByDelta_multiplier__parsesNodeCorrectly()
{
    const auto node = Node::create("- mouse: [ move_by_delta 1.25 ]");
    const auto items = node->as<std::vector<InputActionItem>>();

    QCOMPARE(items.size(), 1);
    QCOMPARE(items[0].mouseMoveRelativeByDelta, static_cast<qreal>(1.25));
}

void TestInputActionNodeParser::mouseMoveRelative__parsesNodeCorrectly()
{
    const auto node = Node::create("- mouse: [ move_by -1.2 1.2 ]");
    const auto items = node->as<std::vector<InputActionItem>>();

    QCOMPARE(items.size(), 1);
    QCOMPARE(items[0].mouseMoveRelative, QPointF(-1.2, 1.2));
}

void TestInputActionNodeParser::mouseWheel__parsesNodeCorrectly()
{
    const auto node = Node::create("- mouse: [ wheel -1.2 1.2 ]");
    const auto items = node->as<std::vector<InputActionItem>>();

    QCOMPARE(items.size(), 1);
    QCOMPARE(items[0].mouseAxis, QPointF(-1.2, 1.2));
}

void TestInputActionNodeParser::invalid__throwsInvalidValueConfigException_data()
{
    QTest::addColumn<QString>("config");
    QTest::addColumn<int>("column");

    QTest::newRow("keyboardKey_invalid") << "- keyboard: [ aa ]" << 14;
    QTest::newRow("keyboardKey_twoSeparatedBySpace") << "- keyboard: [ a a ]" << 14;
    QTest::newRow("keyboardKey_twoSeparatedByTwoPluses") << "- keyboard: [ a++b ]" << 14;
    QTest::newRow("keyboardKey_twoLeadingPluses") << "- keyboard: [ ++a ]" << 15;
    QTest::newRow("keyboardKey_twoTrailingPluses") << "- keyboard: [ a++ ]" << 14;
    QTest::newRow("keyboardKey_twoLeadingMinuses") << "- keyboard: [ --a ]" << 15;
    QTest::newRow("keyboardKey_twoTrailingMinuses") << "- keyboard: [ a-- ]" << 14;
    QTest::newRow("mouseButton_invalid") << "- mouse: [ aa ]" << 11;
    QTest::newRow("mouseButton_twoSeparatedBySpace") << "- mouse: [ left right ]" << 11;
    QTest::newRow("mouseButton_twoSeparatedByTwoPluses") << "- mouse: [ left++right ]" << 11;
    QTest::newRow("mouseButton_combined_twoPluses") << "- mouse: [ left++right ]" << 11;
    QTest::newRow("mouseButton_twoLeadingPluses") << "- mouse: [ ++left ]" << 12;
    QTest::newRow("mouseButton_twoTrailingPluses") << "- mouse: [ left++ ]" << 11;
    QTest::newRow("mouseButton_twoLeadingMinuses") << "- mouse: [ --left ]" << 12;
    QTest::newRow("mouseButton_twoTrailingMinuses") << "- mouse: [ left-- ]" << 11;
    QTest::newRow("mouseMoveByDelta_multiplierNotANumber") << "- mouse: [ move_by_delta x ]" << 25;
    QTest::newRow("mouseMoveRelative_empty") << "- mouse: [ move_by ]" << 11;
    QTest::newRow("mouseMoveRelative_spaces") << "- mouse: [ \"move_by  \" ]" << 11;
    QTest::newRow("mouseMoveRelative_oneArgument") << "- mouse: [ move_by 1 ]" << 19;
    QTest::newRow("mouseMoveRelative_twoSpacesBetweenArguments") << "- mouse: [ move_by 1  1 ]" << 11;
    QTest::newRow("mouseMoveRelative_nonNumericArguments") << "- mouse: [ move_by x x ]" << 11;
    QTest::newRow("mouseWheel_empty") << "- mouse: [ wheel ]" << 11;
    QTest::newRow("mouseWheel_spaces") << "- mouse: [ \"wheel  \" ]" << 11;
    QTest::newRow("mouseWheel_oneArgument") << "- mouse: [ wheel 1 ]" << 17;
    QTest::newRow("mouseWheel_twoSpacesBetweenArguments") << "- mouse: [ wheel 1  1 ]" << 11;
    QTest::newRow("mouseWheel_nonNumericArguments") << "- mouse: [ wheel x x ]" << 11;
}

void TestInputActionNodeParser::invalid__throwsInvalidValueConfigException()
{
    QFETCH(QString, config);
    QFETCH(int, column);

    const auto node = Node::create(config);
    INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<std::vector<InputActionItem>>(), InvalidValueConfigException, 0, column);
}

}

QTEST_MAIN(InputActions::TestInputActionNodeParser)