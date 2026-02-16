#include "TestSetNodeParser.h"
#include <libinputactions/config/ConfigIssue.h>
#include <libinputactions/config/parsers/containers.h>

namespace InputActions
{

void TestSetNodeParser::valid__parsesNodeCorrectly()
{
    const auto node = Node::create("[ 1, 2, 3 ]");
    const auto set = node->as<std::set<uint32_t>>();

    QCOMPARE(set, (std::set<uint32_t>{1, 2, 3}));
}

void TestSetNodeParser::invalid_duplicateItem__throwsDuplicateSetItemConfigException()
{
    const auto node = Node::create("[ 1, 1 ]");

    INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION_SAVE(node->as<std::set<uint32_t>>(), DuplicateSetItemConfigException, 0, 5, e);
    QCOMPARE(e->index(), 1);
}

void TestSetNodeParser::invalid_scalar__throwsInvalidNodeTypeConfigException()
{
    const auto node = Node::create("1");

    INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION_SAVE(node->as<std::set<uint32_t>>(), InvalidNodeTypeConfigException, 0, 0, e);
    QCOMPARE(e->expected(), NodeType::Sequence);
    QCOMPARE(e->actual(), NodeType::Scalar);
}

}

QTEST_MAIN(InputActions::TestSetNodeParser)