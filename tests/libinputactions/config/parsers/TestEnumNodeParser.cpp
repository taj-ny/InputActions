#include "TestEnumNodeParser.h"
#include <libinputactions/config/ConfigIssue.h>
#include <libinputactions/config/Node.h>
#include <libinputactions/config/parsers/enums.h>

namespace InputActions
{

enum class TestEnum
{
    A
};

NODEPARSER_ENUM(TestEnum, "",
                (std::unordered_map<QString, TestEnum>{
                    {"a", TestEnum::A},
                }))

void TestEnumNodeParser::valid__parsesNodeCorrectly()
{
    const auto node = Node::create("a");
    QCOMPARE(node->as<TestEnum>(), TestEnum::A);
}

void TestEnumNodeParser::invalid_differentCase__throwsInvalidValueConfigException()
{
    const auto node = Node::create("A");
    INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<TestEnum>(), InvalidValueConfigException, 0, 0);
}

void TestEnumNodeParser::invalid__throwsInvalidValueConfigException()
{
    const auto node = Node::create("d");
    INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<TestEnum>(), InvalidValueConfigException, 0, 0);
}

}

QTEST_MAIN(InputActions::TestEnumNodeParser)