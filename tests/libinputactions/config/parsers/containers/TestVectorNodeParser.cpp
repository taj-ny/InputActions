#include "TestVectorNodeParser.h"
#include <libinputactions/config/ConfigIssue.h>
#include <libinputactions/config/parsers/containers.h>

namespace InputActions
{

void TestVectorNodeParser::valid__parsesNodeCorrectly()
{
    const auto node = Node::create("[ 1, 2, 3 ]");
    const auto vector = node->as<std::vector<uint32_t>>();

    QCOMPARE(vector, (std::vector<uint32_t>{1, 2, 3}));
}

void TestVectorNodeParser::duplicateItem__parsesNodeCorrectly()
{
    const auto node = Node::create("[ 1, 1 ]");
    const auto vector = node->as<std::vector<uint32_t>>();

    QCOMPARE(vector, (std::vector<uint32_t>{1, 1}));
}

void TestVectorNodeParser::invalid_scalar__throwsInvalidNodeTypeConfigException()
{
    const auto node = Node::create("1");

    INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION_SAVE(node->as<std::vector<uint32_t>>(), InvalidNodeTypeConfigException, 0, 0, e);
    QCOMPARE(e->expected(), NodeType::Sequence);
    QCOMPARE(e->actual(), NodeType::Scalar);
}

}

QTEST_MAIN(InputActions::TestVectorNodeParser)