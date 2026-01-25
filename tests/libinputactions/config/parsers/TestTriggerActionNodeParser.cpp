#include "TestTriggerActionNodeParser.h"
#include <libinputactions/actions/TriggerAction.h>
#include "libinputactions/config/ConfigIssue.h"
#include <libinputactions/config/Node.h>
#include <memory>

namespace InputActions
{

void TestTriggerActionNodeParser::valid__doesNotThrow()
{
    const auto node = Node::load("command: _");

    QVERIFY_THROWS_NO_EXCEPTION(node.as<std::unique_ptr<TriggerAction>>());
}

void TestTriggerActionNodeParser::interval_on_data()
{
    QTest::addColumn<QString>("on");
    QTest::addColumn<bool>("throwsInvalidValueConfigException");

    QTest::addRow("begin__throws") << "begin" << true;
    QTest::addRow("cancel__throws") << "cancel" << true;
    QTest::addRow("end__throws") << "end" << true;
    QTest::addRow("end_cancel__throws") << "end_cancel" << true;
    QTest::addRow("tick__doesNotThrow") << "tick" << false;
    QTest::addRow("update__doesNotThrow") << "update" << false;
}

void TestTriggerActionNodeParser::interval_on()
{
    QFETCH(QString, on);
    QFETCH(bool, throwsInvalidValueConfigException);

    const auto node = Node::load(QString(R"(
        on: %1
        interval: 1
        command: _
    )").arg(on));

    if (throwsInvalidValueConfigException) {
        QVERIFY_THROWS_EXCEPTION(InvalidValueConfigException, node.as<std::unique_ptr<TriggerAction>>());
    } else {
        QVERIFY_THROWS_NO_EXCEPTION(node.as<std::unique_ptr<TriggerAction>>());
    }
}

void TestTriggerActionNodeParser::threshold_on_data()
{
    QTest::addColumn<QString>("on");
    QTest::addColumn<bool>("throwsInvalidValueConfigException");

    QTest::addRow("begin__throws") << "begin" << true;
    QTest::addRow("cancel__doesNotThrow") << "cancel" << false;
    QTest::addRow("end__doesNotThrow") << "end" << false;
    QTest::addRow("end_cancel__doesNotThrow") << "end_cancel" << false;
    QTest::addRow("tick__doesNotThrow") << "tick" << false;
    QTest::addRow("update__doesNotThrow") << "update" << false;
}

void TestTriggerActionNodeParser::threshold_on()
{
    QFETCH(QString, on);
    QFETCH(bool, throwsInvalidValueConfigException);

    const auto node = Node::load(QString(R"(
        on: %1
        threshold: 1
        command: _
    )").arg(on));

    if (throwsInvalidValueConfigException) {
        QVERIFY_THROWS_EXCEPTION(InvalidValueConfigException, node.as<std::unique_ptr<TriggerAction>>());
    } else {
        QVERIFY_THROWS_NO_EXCEPTION(node.as<std::unique_ptr<TriggerAction>>());
    }
}

}

QTEST_MAIN(InputActions::TestTriggerActionNodeParser)
#include "TestTriggerActionNodeParser.moc"
