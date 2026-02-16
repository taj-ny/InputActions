#include "TestRangeNodeParser.h"
#include <libinputactions/Range.h>
#include <libinputactions/config/Node.h>

namespace InputActions
{

void TestRangeNodeParser::valid__parsesNodeCorrectly()
{
    const auto node = Node::create("12.34-43.21");
    const auto range = node->as<Range<qreal>>();

    QCOMPARE(range.min(), 12.34);
    QCOMPARE(range.max(), 43.21);
}

void TestRangeNodeParser::singularValue__parsesNodeCorrectly()
{
    const auto node = Node::create("1");
    const auto range = node->as<Range<qreal>>();

    QCOMPARE(range.min(), 1);
    QCOMPARE(range.max(), std::nullopt);
}

}

QTEST_MAIN(InputActions::TestRangeNodeParser)