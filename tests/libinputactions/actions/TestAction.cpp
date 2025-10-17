#include "TestAction.h"
#include <libinputactions/actions/Action.h>
#include <libinputactions/actions/ActionExecutor.h>

namespace libinputactions
{

void TestAction::init()
{
    m_action = std::make_shared<Action>();
}

void TestAction::canExecute_noLimit()
{
    QVERIFY(m_action->canExecute());
    g_actionExecutor->execute(m_action);
    QVERIFY(m_action->canExecute());
}

void TestAction::canExecute_withLimit()
{
    m_action->m_executionLimit = 1;
    QVERIFY(m_action->canExecute());
    g_actionExecutor->execute(m_action);
    QVERIFY(!m_action->canExecute());

    m_action->reset();
    QVERIFY(m_action->canExecute());
    g_actionExecutor->execute(m_action);
    QVERIFY(!m_action->canExecute());
}

}

QTEST_MAIN(libinputactions::TestAction)
#include "TestAction.moc"