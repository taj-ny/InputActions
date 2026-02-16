#include "TestConditionGroupNodeParser.h"
#include <libinputactions/conditions/ConditionGroup.h>
#include <libinputactions/config/Node.h>

namespace InputActions
{

void TestConditionGroupNodeParser::all__parsesNodeCorrectly()
{
    const auto node = Node::create("all: []");
    const auto condition = std::dynamic_pointer_cast<ConditionGroup>(node->as<std::shared_ptr<Condition>>());

    QVERIFY(condition);
    QCOMPARE(condition->mode(), ConditionGroupMode::All);
    QCOMPARE(condition->conditions().size(), 0);
}

void TestConditionGroupNodeParser::any__parsesNodeCorrectly()
{
    const auto node = Node::create("any: []");
    const auto condition = std::dynamic_pointer_cast<ConditionGroup>(node->as<std::shared_ptr<Condition>>());

    QVERIFY(condition);
    QCOMPARE(condition->mode(), ConditionGroupMode::Any);
    QCOMPARE(condition->conditions().size(), 0);
}

void TestConditionGroupNodeParser::none__parsesNodeCorrectly()
{
    const auto node = Node::create("none: []");
    const auto condition = std::dynamic_pointer_cast<ConditionGroup>(node->as<std::shared_ptr<Condition>>());

    QVERIFY(condition);
    QCOMPARE(condition->mode(), ConditionGroupMode::None);
    QCOMPARE(condition->conditions().size(), 0);
}

void TestConditionGroupNodeParser::list__parsesNodeAsAllGroup()
{
    const auto node = Node::create("[]");
    const auto condition = std::dynamic_pointer_cast<ConditionGroup>(node->as<std::shared_ptr<Condition>>());

    QVERIFY(condition);
    QCOMPARE(condition->mode(), ConditionGroupMode::All);
    QCOMPARE(condition->conditions().size(), 0);
}

void TestConditionGroupNodeParser::nested__parsesNodeCorrectly()
{
    const auto node = Node::create(R"(
        all:
          - any:
              - none: []
    )");
    const auto condition = std::dynamic_pointer_cast<ConditionGroup>(node->as<std::shared_ptr<Condition>>());

    QVERIFY(condition);
    QCOMPARE(condition->mode(), ConditionGroupMode::All);
    QCOMPARE(condition->conditions().size(), 1);

    const auto any = std::dynamic_pointer_cast<ConditionGroup>(condition->conditions()[0]);
    QVERIFY(any);
    QCOMPARE(any->mode(), ConditionGroupMode::Any);
    QCOMPARE(any->conditions().size(), 1);

    const auto none = std::dynamic_pointer_cast<ConditionGroup>(any->conditions()[0]);
    QVERIFY(none);
    QCOMPARE(none->mode(), ConditionGroupMode::None);
    QCOMPARE(none->conditions().size(), 0);
}

void TestConditionGroupNodeParser::invalid_scalarAsChild__throwsInvalidNodeTypeConfigException()
{
    const auto node = Node::create("all: a");

    INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION_SAVE(node->as<std::shared_ptr<Condition>>(), InvalidNodeTypeConfigException, 0, 5, e);
    QCOMPARE(e->expected(), NodeType::Sequence);
    QCOMPARE(e->actual(), NodeType::Scalar);
}

}

QTEST_MAIN(InputActions::TestConditionGroupNodeParser)