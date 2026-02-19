#include "TestQRegularExpressionNodeParser.h"
#include <QRegularExpression>
#include <libinputactions/config/ConfigIssue.h>
#include <libinputactions/config/Node.h>

namespace InputActions
{

void TestQRegularExpressionNodeParser::valid__parsesNodeCorrectly()
{
    const auto node = Node::create("\"[a]\"");
    const auto regex = node->as<QRegularExpression>();

    QVERIFY(regex.isValid());
    QCOMPARE(regex.pattern(), "[a]");
}

void TestQRegularExpressionNodeParser::invalid__throwsInvalidValueConfigException()
{
    const auto node = Node::create("\"[a\"");
    INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<QRegularExpression>(), InvalidValueConfigException, 0, 0);
}

}

QTEST_MAIN(InputActions::TestQRegularExpressionNodeParser)