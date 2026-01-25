#include "TestInputActionNodeParser.h"
#include "libinputactions/config/ConfigIssue.h"
#include <libinputactions/actions/InputAction.h>
#include <libinputactions/config/Node.h>
#include <linux/input-event-codes.h>
#include <memory>

namespace InputActions
{

void TestInputActionNodeParser::keyboardKey_twoSeparate__parsedCorrectly()
{
    const auto node = Node::load("- keyboard: [ a, b ]");
    const auto actions = node.as<std::vector<InputAction::Item>>();

    QCOMPARE(actions.size(), 4);
    QCOMPARE(actions[0].keyboardPress, KEY_A);
    QCOMPARE(actions[1].keyboardRelease, KEY_A);
    QCOMPARE(actions[2].keyboardPress, KEY_B);
    QCOMPARE(actions[3].keyboardRelease, KEY_B);
}

void TestInputActionNodeParser::keyboardKey_twoCombined__parsedCorrectly()
{
    const auto node = Node::load("- keyboard: [ a+b ]");
    const auto actions = node.as<std::vector<InputAction::Item>>();

    QCOMPARE(actions.size(), 4);
    QCOMPARE(actions[0].keyboardPress, KEY_A);
    QCOMPARE(actions[1].keyboardPress, KEY_B);
    QCOMPARE(actions[2].keyboardRelease, KEY_B);
    QCOMPARE(actions[3].keyboardRelease, KEY_A);
}

void TestInputActionNodeParser::keyboardKeyPress_two__parsedCorrectly()
{
    const auto node = Node::load("- keyboard: [ +a, +b ]");
    const auto actions = node.as<std::vector<InputAction::Item>>();

    QCOMPARE(actions.size(), 2);
    QCOMPARE(actions[0].keyboardPress, KEY_A);
    QCOMPARE(actions[1].keyboardPress, KEY_B);
}

void TestInputActionNodeParser::keyboardKeyRelease_two__parsedCorrectly()
{
    const auto node = Node::load("- keyboard: [ -a, -b ]");
    const auto actions = node.as<std::vector<InputAction::Item>>();

    QCOMPARE(actions.size(), 2);
    QCOMPARE(actions[0].keyboardRelease, KEY_A);
    QCOMPARE(actions[1].keyboardRelease, KEY_B);
}

void TestInputActionNodeParser::keyboardText__parsedCorrectly()
{
    const auto node = Node::load("- keyboard: [ text: aaa ]");
    const auto actions = node.as<std::vector<InputAction::Item>>();

    QCOMPARE(actions.size(), 1);
    QCOMPARE(actions[0].keyboardText.get(), "aaa");
}

void TestInputActionNodeParser::keyboardText_command__parsedCorrectly()
{
    const auto node = Node::load("- keyboard: [ text: { command: echo a } ]");
    const auto actions = node.as<std::vector<InputAction::Item>>();

    QCOMPARE(actions.size(), 1);
    QCOMPARE(actions[0].keyboardText.get(), "a\n");
}

void TestInputActionNodeParser::mouseAxis__parsedCorrectly()
{
    const auto node = Node::load("- mouse: [ wheel -10 10 ]");
    const auto actions = node.as<std::vector<InputAction::Item>>();

    QCOMPARE(actions.size(), 1);
    QCOMPARE(actions[0].mouseAxis, QPointF(-10, 10));
}

void TestInputActionNodeParser::mouseButton_twoSeparate__parsedCorrectly()
{
    const auto node = Node::load("- mouse: [ left, right ]");
    const auto actions = node.as<std::vector<InputAction::Item>>();

    QCOMPARE(actions.size(), 4);
    QCOMPARE(actions[0].mousePress, BTN_LEFT);
    QCOMPARE(actions[1].mouseRelease, BTN_LEFT);
    QCOMPARE(actions[2].mousePress, BTN_RIGHT);
    QCOMPARE(actions[3].mouseRelease, BTN_RIGHT);
}

void TestInputActionNodeParser::mouseButton_twoCombined__parsedCorrectly()
{
    const auto node = Node::load("- mouse: [ left+right ]");
    const auto actions = node.as<std::vector<InputAction::Item>>();

    QCOMPARE(actions.size(), 4);
    QCOMPARE(actions[0].mousePress, BTN_LEFT);
    QCOMPARE(actions[1].mousePress, BTN_RIGHT);
    QCOMPARE(actions[2].mouseRelease, BTN_RIGHT);
    QCOMPARE(actions[3].mouseRelease, BTN_LEFT);
}

void TestInputActionNodeParser::mouseButtonPress_two__parsedCorrectly()
{
    const auto node = Node::load("- mouse: [ +left, +right ]");
    const auto actions = node.as<std::vector<InputAction::Item>>();

    QCOMPARE(actions.size(), 2);
    QCOMPARE(actions[0].mousePress, BTN_LEFT);
    QCOMPARE(actions[1].mousePress, BTN_RIGHT);
}

void TestInputActionNodeParser::mouseButtonRelease_two__parsedCorrectly()
{
    const auto node = Node::load("- mouse: [ -left, -right ]");
    const auto actions = node.as<std::vector<InputAction::Item>>();

    QCOMPARE(actions.size(), 2);
    QCOMPARE(actions[0].mouseRelease, BTN_LEFT);
    QCOMPARE(actions[1].mouseRelease, BTN_RIGHT);
}

void TestInputActionNodeParser::mouseMoveAbsolute__parsedCorrectly()
{
    const auto node = Node::load("- mouse: [ move_to -10 10 ]");
    const auto actions = node.as<std::vector<InputAction::Item>>();

    QCOMPARE(actions.size(), 1);
    QCOMPARE(actions[0].mouseMoveAbsolute, QPointF(-10, 10));
}

void TestInputActionNodeParser::mouseMoveByDelta__parsedCorrectly()
{
    const auto node = Node::load("- mouse: [ move_by_delta ]");
    const auto actions = node.as<std::vector<InputAction::Item>>();

    QCOMPARE(actions.size(), 1);
    QVERIFY(actions[0].mouseMoveRelativeByDelta);
}

void TestInputActionNodeParser::mouseMoveRelative__parsedCorrectly()
{
    const auto node = Node::load("- mouse: [ move_by -10 10 ]");
    const auto actions = node.as<std::vector<InputAction::Item>>();

    QCOMPARE(actions.size(), 1);
    QCOMPARE(actions[0].mouseMoveRelative, QPointF(-10, 10));
}

void TestInputActionNodeParser::invalid__throwsInvalidValueConfigException_data()
{
    QTest::addColumn<QString>("config");

    QTest::newRow("keyboardKey_invalid") << "- keyboard: [ aa ]";
    QTest::newRow("keyboardKey_twoSeparatedBySpace") << "- keyboard: [ a a ]";
    QTest::newRow("keyboardKey_twoSeparatedByTwoPluses") << "- keyboard: [ a++b ]";
    QTest::newRow("keyboardKey_twoLeadingPluses") << "- keyboard: [ ++a ]";
    QTest::newRow("keyboardKey_twoTrailingPluses") << "- keyboard: [ a++ ]";
    QTest::newRow("keyboardKey_twoLeadingMinuses") << "- keyboard: [ --a ]";
    QTest::newRow("keyboardKey_twoTrailingMinuses") << "- keyboard: [ a-- ]";
    QTest::newRow("mouseButton_invalid") << "- mouse: [ aa ]";
    QTest::newRow("mouseButton_twoSeparatedBySpace") << "- mouse: [ left right ]";
    QTest::newRow("mouseButton_twoSeparatedByTwoPluses") << "- mouse: [ left++right ]";
    QTest::newRow("mouseButton_combined_twoPluses") << "- mouse: [ left++right ]";
    QTest::newRow("mouseButton_twoLeadingPluses") << "- mouse: [ ++left ]";
    QTest::newRow("mouseButton_twoTrailingPluses") << "- mouse: [ left++ ]";
    QTest::newRow("mouseButton_twoLeadingMinuses") << "- mouse: [ --left ]";
    QTest::newRow("mouseButton_twoTrailingMinuses") << "- mouse: [ left-- ]";
    QTest::newRow("mouseMoveRelative_empty") << "- mouse: [ move_by ]";
    QTest::newRow("mouseMoveRelative_spaces") << "- mouse: [ move_by   ]";
    QTest::newRow("mouseMoveRelative_oneArgument") << "- mouse: [ move_by 1 ]";
    QTest::newRow("mouseMoveRelative_twoSpacesBetweenArguments") << "- mouse: [ move_by 1  1 ]";
    QTest::newRow("mouseMoveRelative_nonNumericArguments") << "- mouse: [ move_by a a ]";
}

void TestInputActionNodeParser::invalid__throwsInvalidValueConfigException()
{
    QFETCH(QString, config);

    const auto node = Node::load(config);
    QVERIFY_THROWS_EXCEPTION(InvalidValueConfigException, node.as<std::vector<InputAction::Item>>());
}

}

QTEST_MAIN(InputActions::TestInputActionNodeParser)
#include "TestInputActionNodeParser.moc"
