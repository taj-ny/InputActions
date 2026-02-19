#include "TestFlagsNodeParser.h"
#include <libinputactions/config/ConfigIssue.h>
#include <libinputactions/config/Node.h>
#include <libinputactions/config/parsers/flags.h>

namespace InputActions
{

void TestFlagsNodeParser::empty__parsesNodeCorrectly()
{
    const auto node = Node::create("[]");
    QCOMPARE(node->as<Qt::KeyboardModifiers>(), Qt::KeyboardModifier::NoModifier);
}

void TestFlagsNodeParser::oneItem__parsesNodeCorrectly()
{
    const auto node = Node::create("[ ctrl ]");
    QCOMPARE(node->as<Qt::KeyboardModifiers>(), Qt::KeyboardModifier::ControlModifier);
}

void TestFlagsNodeParser::twoItems__parsesNodeCorrectly()
{
    const auto node = Node::create("[ ctrl, meta ]");
    QCOMPARE(node->as<Qt::KeyboardModifiers>(), Qt::KeyboardModifier::ControlModifier | Qt::KeyboardModifier::MetaModifier);
}

void TestFlagsNodeParser::invalid_duplicateItem__throwsInvalidNodeTypeConfigException()
{
    const auto node = Node::create("[ meta, meta ]");

    INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION_SAVE(node->as<Qt::KeyboardModifiers>(), DuplicateSetItemConfigException, 0, 8, e);
    QCOMPARE(e->index(), 1);
}

void TestFlagsNodeParser::invalid_scalar__throwsInvalidNodeTypeConfigException()
{
    const auto node = Node::create("meta");

    INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION_SAVE(node->as<Qt::KeyboardModifiers>(), InvalidNodeTypeConfigException, 0, 0, e);
    QCOMPARE(e->expected(), NodeType::Sequence);
    QCOMPARE(e->actual(), NodeType::Scalar);
}

}

QTEST_MAIN(InputActions::TestFlagsNodeParser)