#include "test_condition.h"

namespace libinputactions
{

void TestCondition::init()
{
    m_condition = std::make_shared<Condition>(m_normalWindowProvider);
}

void TestCondition::isSatisfied_noActiveWindow_returnsFalse()
{
    QVERIFY(!Condition(m_noActiveWindowProvider).isSatisfied());
}

void TestCondition::isSatisfied_noSubConditions_returnsTrue()
{
    QVERIFY(Condition(m_normalWindowProvider).isSatisfied());
}

void TestCondition::isSatisfied_negatedAndNoSubConditions_returnsTrue()
{
    m_condition->setNegate(true);

    QVERIFY(m_condition->isSatisfied());
}

void TestCondition::isWindowClassRegexSubConditionSatisfied_subConditionNotSet_returnsTrue()
{
    QVERIFY(m_condition->isWindowClassRegexSubConditionSatisfied(m_normalWindow));
}

void TestCondition::isWindowClassRegexSubConditionSatisfied_negatedAndSubConditionNotSet_returnsTrue()
{
    m_condition->setNegate(true);

    QVERIFY(m_condition->isWindowClassRegexSubConditionSatisfied(m_normalWindow));
}

void TestCondition::isWindowClassRegexSubConditionSatisfied_data()
{
    QTest::addColumn<bool>("negate");
    QTest::addColumn<QString>("regex");
    QTest::addColumn<QString>("resourceClass");
    QTest::addColumn<QString>("resourceName");
    QTest::addColumn<bool>("result");

    QTest::newRow("resource class only") << false << s_windowClass << s_windowClass << "" << true;
    QTest::newRow("resource name only") << false << s_windowClass << "" << s_windowClass << true;
    QTest::newRow("no match") << false << s_windowClass << "" << "" << false;
    QTest::newRow("negated match") << true << s_windowClass << s_windowClass << s_windowClass << false;
    QTest::newRow("negated no match") << true << s_windowClass << "" << "" << true;
}

void TestCondition::isWindowClassRegexSubConditionSatisfied()
{
    QFETCH(bool, negate);
    QFETCH(QString, regex);
    QFETCH(QString, resourceClass);
    QFETCH(QString, resourceName);
    QFETCH(bool, result);
    const WindowInfo windowData(s_windowCaption, resourceClass, resourceName, WindowState::Unimportant);

    m_condition->setNegate(negate);
    m_condition->setWindowClass(QRegularExpression(regex));

    QCOMPARE(m_condition->isWindowClassRegexSubConditionSatisfied(windowData), result);
}

void TestCondition::isWindowStateSubConditionSatisfied_subConditionNotSet_returnsTrue()
{
    QVERIFY(m_condition->isWindowStateSubConditionSatisfied(m_normalWindow));
}

void TestCondition::isWindowStateSubConditionSatisfied_negatedAndSubConditionNotSet_returnsTrue()
{
    m_condition->setNegate(true);

    QVERIFY(m_condition->isWindowStateSubConditionSatisfied(m_normalWindow));
}

void TestCondition::isWindowStateSubConditionSatisfied_data()
{
    QTest::addColumn<bool>("negate");
    QTest::addColumn<int>("conditionWindowState");
    QTest::addColumn<int>("windowState");
    QTest::addColumn<bool>("result");

    QTest::newRow("matches one") << false << static_cast<int>(WindowState::Maximized)
        << static_cast<int>(WindowState::Maximized) << true;
    QTest::newRow("matches all") << false << static_cast<int>(WindowState::Maximized | WindowState::Fullscreen)
        << static_cast<int>(WindowState::Maximized | WindowState::Fullscreen) << true;
    QTest::newRow("no match") << false << static_cast<int>(WindowState::Maximized) << static_cast<int>(WindowState::Fullscreen) << false;
    QTest::newRow("negated matches one") << true << static_cast<int>(WindowState::Maximized)
        << static_cast<int>(WindowState::Maximized) << false;
    QTest::newRow("negated matches all") << true << static_cast<int>(WindowState::Maximized | WindowState::Fullscreen)
        << static_cast<int>(WindowState::Maximized | WindowState::Fullscreen) << false;
    QTest::newRow("negated no match") << true << static_cast<int>(WindowState::Maximized) << static_cast<int>(WindowState::Fullscreen) << true;
}

void TestCondition::isWindowStateSubConditionSatisfied()
{
    QFETCH(bool, negate);
    QFETCH(int, conditionWindowState);
    QFETCH(int, windowState);
    QFETCH(bool, result);
    const WindowInfo windowData(s_windowCaption, s_windowClass, s_windowClass, static_cast<WindowState>(windowState));

    m_condition->setNegate(negate);
    m_condition->setWindowState(static_cast<WindowState>(conditionWindowState));

    QCOMPARE(m_condition->isWindowStateSubConditionSatisfied(windowData), result);
}

}

QTEST_MAIN(libinputactions::TestCondition)
#include "test_condition.moc"