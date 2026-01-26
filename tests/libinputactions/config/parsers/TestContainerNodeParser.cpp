#include "TestContainerNodeParser.h"
#include "libinputactions/config/ConfigIssue.h"
#include <libinputactions/config/parsers/containers.h>

namespace InputActions
{

void TestContainerNodeParser::set__parsedCorrectly()
{
    const auto node = Node::load("[ 1, 2, 3 ]");
    const auto set = node.as<std::set<uint32_t>>();

    QCOMPARE(set, (std::set<uint32_t>{1, 2, 3}));
}

void TestContainerNodeParser::set__duplicateElement__throwsDuplicateSetItemConfigException()
{
    const auto node = Node::load("[ 1, 1 ]");

    try {
        node.as<std::set<uint32_t>>();
        QFAIL("Expected DuplicateSetItemConfigException to be thrown.");
    } catch (const DuplicateSetItemConfigException &e) {
        QCOMPARE(e.index(), 1);
    }
}

void TestContainerNodeParser::set__scalar__throwsInvalidNodeTypeConfigException()
{
    const auto node = Node::load("1");

    try {
        node.as<std::set<uint32_t>>();
        QFAIL("Expected InvalidNodeTypeConfigException to be thrown.");
    } catch (const InvalidNodeTypeConfigException &e) {
        QCOMPARE(e.expected(), NodeType::Sequence);
        QCOMPARE(e.actual(), NodeType::Scalar);
    }
}

void TestContainerNodeParser::vector__parsedCorrectly()
{
    const auto node = Node::load("[ 1, 2, 3 ]");
    const auto vector = node.as<std::vector<uint32_t>>();

    QCOMPARE(vector, (std::vector<uint32_t>{1, 2, 3}));
}

void TestContainerNodeParser::vector__duplicateElement__parsedCorrectly()
{
    const auto node = Node::load("[ 1, 1 ]");
    const auto vector = node.as<std::vector<uint32_t>>();

    QCOMPARE(vector, (std::vector<uint32_t>{1, 1}));
}

void TestContainerNodeParser::vector__scalar__throwsInvalidNodeTypeConfigException()
{
    const auto node = Node::load("1");

    try {
        node.as<std::vector<uint32_t>>();
        QFAIL("Expected InvalidNodeTypeConfigException to be thrown.");
    } catch (const InvalidNodeTypeConfigException &e) {
        QCOMPARE(e.expected(), NodeType::Sequence);
        QCOMPARE(e.actual(), NodeType::Scalar);
    }
}

}

QTEST_MAIN(InputActions::TestContainerNodeParser)
#include "TestContainerNodeParser.moc"