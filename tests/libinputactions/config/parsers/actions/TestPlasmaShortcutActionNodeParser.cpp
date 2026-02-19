#include "TestPlasmaShortcutActionNodeParser.h"
#include <libinputactions/actions/PlasmaGlobalShortcutAction.h>
#include <libinputactions/config/ConfigIssue.h>
#include <libinputactions/config/Node.h>

namespace InputActions
{

void TestPlasmaShortcutActionNodeParser::valid__parsesNodeCorrectly()
{
    const auto node = Node::create("plasma_shortcut: a,b");
    const auto action = std::dynamic_pointer_cast<PlasmaGlobalShortcutAction>(node->as<std::shared_ptr<Action>>());

    QCOMPARE(action->component(), "a");
    QCOMPARE(action->shortcut(), "b");
}

void TestPlasmaShortcutActionNodeParser::invalid__throwsInvalidValueConfigException()
{
    const auto node = Node::create("plasma_shortcut: _");
    INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<std::shared_ptr<Action>>(), InvalidValueConfigException, 0, 17);
}

}

QTEST_MAIN(InputActions::TestPlasmaShortcutActionNodeParser)