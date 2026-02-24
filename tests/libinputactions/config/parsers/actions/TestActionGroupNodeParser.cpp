#include "TestActionGroupNodeParser.h"
#include <libinputactions/actions/ActionGroup.h>
#include <libinputactions/actions/SleepAction.h>
#include <libinputactions/config/Node.h>

namespace InputActions
{

void TestActionGroupNodeParser::one__parsesNodeCorrectly()
{
    const auto node = Node::create(R"(
        one:
          - sleep: 1
          - sleep: 2
    )");
    const auto action = node->as<std::unique_ptr<Action>>();

    const auto *actionGroup = dynamic_cast<const ActionGroup *>(action.get());
    QVERIFY(actionGroup);
    QCOMPARE(actionGroup->mode(), ActionGroupExecutionMode::First);

    const auto actions = actionGroup->actions();
    QCOMPARE(actions.size(), 2);

    const auto *action1 = dynamic_cast<const SleepAction *>(actions[0]);
    QVERIFY(action1);
    QCOMPARE(action1->time(), std::chrono::milliseconds(1));

    const auto *action2 = dynamic_cast<const SleepAction *>(actions[1]);
    QVERIFY(action2);
    QCOMPARE(action2->time(), std::chrono::milliseconds(2));
}

}

QTEST_MAIN(InputActions::TestActionGroupNodeParser)