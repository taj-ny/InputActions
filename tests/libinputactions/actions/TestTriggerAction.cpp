#include "TestTriggerAction.h"

#include "utils.h"

using namespace ::testing;

namespace libinputactions
{

void TestTriggerAction::init()
{
    m_action = std::make_unique<MockTriggerAction>();
}

void TestTriggerAction::canExecute_data()
{
    QTest::addColumn<std::optional<bool>>("condition");
    QTest::addColumn<std::optional<bool>>("threshold");
    QTest::addColumn<bool>("result");

    QTest::newRow("no condition, no threshold") << std::optional<bool>() << std::optional<bool>() << true;
    QTest::newRow("condition true, no threshold") << std::optional<bool>(true) << std::optional<bool>() << true;
    QTest::newRow("condition false, no threshold") << std::optional<bool>(false) << std::optional<bool>() << false;
    QTest::newRow("no condition, threshold true") << std::optional<bool>() << std::optional<bool>(true) << true;
    QTest::newRow("no condition, threshold false") << std::optional<bool>() << std::optional<bool>(false) << false;
    QTest::newRow("condition true, threshold true") << std::optional<bool>(true) << std::optional<bool>(true) << true;
    QTest::newRow("condition false, threshold false") << std::optional<bool>(false) << std::optional<bool>(false) << false;
    QTest::newRow("condition false, threshold true") << std::optional<bool>(false) << std::optional<bool>(true) << false;
    QTest::newRow("condition true, threshold false") << std::optional<bool>(true) << std::optional<bool>(false) << false;
}

void TestTriggerAction::canExecute()
{
    QFETCH(std::optional<bool>, condition);
    QFETCH(std::optional<bool>, threshold);
    QFETCH(bool, result);

    if (condition) {
        m_action->setCondition(makeCondition(*condition));
    }
    if (threshold) {
        m_action->setThreshold(*threshold ? Range<qreal>(0) : Range<qreal>(1));
    }

    QCOMPARE(m_action->TriggerAction::canExecute(), result);
}

void TestTriggerAction::tryExecute_canExecute_executes()
{
    ON_CALL(*m_action, canExecute())
        .WillByDefault(Return(true));
    EXPECT_CALL(*m_action, execute())
        .Times(Exactly(1));

    m_action->TriggerAction::tryExecute();

    QVERIFY(Mock::VerifyAndClearExpectations(m_action.get()));
}

void TestTriggerAction::tryExecute_cantExecute_doesntExecute()
{
    ON_CALL(*m_action, canExecute())
        .WillByDefault(Return(false));
    EXPECT_CALL(*m_action, execute())
        .Times(Exactly(0));

    m_action->TriggerAction::tryExecute();

    QVERIFY(Mock::VerifyAndClearExpectations(m_action.get()));
}

void TestTriggerAction::gestureStarted_data()
{
    QTest::addColumn<On>("on");
    QTest::addColumn<bool>("executes");

    QTest::newRow("begin") << On::Begin << true;
    QTest::newRow("update") << On::Update << false;
    QTest::newRow("end") << On::End << false;
    QTest::newRow("cancel") << On::Cancel << false;
    QTest::newRow("end or cancel") << On::EndCancel << false;
}

void TestTriggerAction::gestureStarted()
{
    QFETCH(On, on);
    QFETCH(bool, executes);

    EXPECT_CALL(*m_action, tryExecute())
        .Times(Exactly(executes ? 1 : 0));

    m_action->setOn(on);
    m_action->TriggerAction::triggerStarted();

    QVERIFY(Mock::VerifyAndClearExpectations(m_action.get()));
}

void TestTriggerAction::gestureUpdated_data()
{
    QTest::addColumn<std::vector<qreal>>("deltas");
    QTest::addColumn<ActionInterval>("interval");
    QTest::addColumn<int>("executions");

    ActionInterval interval{};
    QTest::newRow("zeroes") << std::vector<qreal>{0, 0, 0} << interval << 3;
    interval.setValue(2);
    QTest::newRow("accumulation") << std::vector<qreal>{1, 1, 1, 1} << interval << 2;
    QTest::newRow("multiple executions") << std::vector<qreal>{4, 4} << interval << 4;
    QTest::newRow("direction change (any)") << std::vector<qreal>{-4, 1, -4, 1} << interval << 4;
    interval.setDirection(IntervalDirection::Positive);
    QTest::newRow("direction change (positive)") << std::vector<qreal>{-4, 1, -4, 1} << interval << 0;
    interval.setDirection(IntervalDirection::Negative);
    QTest::newRow("direction change (negative)") << std::vector<qreal>{4, -1, 4, -1} << interval << 0;
}

void TestTriggerAction::gestureUpdated()
{
    QFETCH(std::vector<qreal>, deltas);
    QFETCH(ActionInterval, interval);
    QFETCH(int, executions);

    EXPECT_CALL(*m_action, tryExecute())
        .Times(Exactly(executions));

    m_action->setOn(On::Update);
    m_action->setRepeatInterval(interval);

    for (const auto &delta : deltas) {
        m_action->TriggerAction::triggerUpdated(delta, {});
    }

    QVERIFY(Mock::VerifyAndClearExpectations(m_action.get()));
}

void TestTriggerAction::gestureEnded_data()
{
    QTest::addColumn<On>("on");
    QTest::addColumn<bool>("executes");

    QTest::newRow("begin") << On::Begin << false;
    QTest::newRow("update") << On::Update << false;
    QTest::newRow("end") << On::End << true;
    QTest::newRow("cancel") << On::Cancel << false;
    QTest::newRow("end or cancel") << On::EndCancel << true;
}

void TestTriggerAction::gestureEnded()
{
    QFETCH(On, on);
    QFETCH(bool, executes);

    EXPECT_CALL(*m_action, tryExecute)
        .Times(Exactly(executes ? 1 : 0));

    m_action->setOn(on);
    m_action->TriggerAction::triggerEnded();

    QVERIFY(Mock::VerifyAndClearExpectations(m_action.get()));
}

void TestTriggerAction::gestureCancelled_data()
{
    QTest::addColumn<On>("on");
    QTest::addColumn<bool>("executes");

    QTest::newRow("begin") << On::Begin << false;
    QTest::newRow("update") << On::Update << false;
    QTest::newRow("end") << On::End << false;
    QTest::newRow("cancel") << On::Cancel << true;
    QTest::newRow("end or cancel") << On::EndCancel << true;
}

void TestTriggerAction::gestureCancelled()
{
    QFETCH(On, on);
    QFETCH(bool, executes);

    EXPECT_CALL(*m_action, tryExecute)
        .Times(Exactly(executes ? 1 : 0));

    m_action->setOn(on);
    m_action->TriggerAction::triggerCancelled();

    QVERIFY(Mock::VerifyAndClearExpectations(m_action.get()));
}

}

QTEST_MAIN(libinputactions::TestTriggerAction)
#include "TestTriggerAction.moc"