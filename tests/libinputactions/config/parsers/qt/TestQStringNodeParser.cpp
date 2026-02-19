#include "TestQStringNodeParser.h"
#include <libinputactions/config/Node.h>

namespace InputActions
{

void TestQStringNodeParser::valid__parsesNodeCorrectly()
{
    const auto node = Node::create("aaa123");
    QCOMPARE(node->as<QString>(), "aaa123");
}

}

QTEST_MAIN(InputActions::TestQStringNodeParser)