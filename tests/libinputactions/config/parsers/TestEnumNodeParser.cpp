#include "TestEnumNodeParser.h"
#include "libinputactions/config/ConfigIssue.h"
#include <libinputactions/config/Node.h>
#include <libinputactions/config/parsers/enums.h>

namespace InputActions
{

enum class TestEnum
{
    A
};

NODEPARSER_ENUM(TestEnum, "", (std::unordered_map<QString, TestEnum>{
    {"a", TestEnum::A},
}))

void TestEnumNodeParser::valid__parsedCorrectly()
{
    const auto node = Node::load("a");
    QCOMPARE(node.as<TestEnum>(), TestEnum::A);
}

void TestEnumNodeParser::invalid_differentCase__throwsInvalidValueConfigException()
{
    const auto node = Node::load("A");
    QVERIFY_THROWS_EXCEPTION(InvalidValueConfigException, node.as<TestEnum>());
}

void TestEnumNodeParser::invalid__throwsInvalidValueConfigException()
{
    const auto node = Node::load("d");
    QVERIFY_THROWS_EXCEPTION(InvalidValueConfigException, node.as<TestEnum>());
}

}

QTEST_MAIN(InputActions::TestEnumNodeParser)
#include "TestEnumNodeParser.moc"