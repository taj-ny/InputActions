#include "TestQPointFNodeParser.h"
#include <libinputactions/config/Node.h>

namespace InputActions
{

void TestQPointFNodeParser::valid__parsesNodeCorrectly()
{
    const auto node = Node::create("12.34,43.21");
    const auto point = node->as<QPointF>();

    QCOMPARE(point, QPointF(12.34, 43.21));
}

void TestQPointFNodeParser::invalid__throwsInvalidValueConfigException()
{
    const auto node = Node::create("12.34");
    INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<QPointF>(), InvalidValueConfigException, 0, 0);
}

}

QTEST_MAIN(InputActions::TestQPointFNodeParser)