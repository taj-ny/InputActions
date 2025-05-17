#include "test_motiontrigger.h"

namespace libinputactions
{

void TestMotionTrigger::init()
{
    m_motionTrigger = std::make_unique<MotionTrigger>();
    m_event = {};
}

void TestMotionTrigger::canUpdate_data()
{
    QTest::addColumn<TriggerSpeed>("triggerSpeed");
    QTest::addColumn<TriggerSpeed>("eventSpeed");
    QTest::addColumn<bool>("result");

    QTest::addRow("any, any") << TriggerSpeed::Any << TriggerSpeed::Any << true;
    QTest::addRow("any, fast") << TriggerSpeed::Any << TriggerSpeed::Fast << true;
    QTest::addRow("any, slow") << TriggerSpeed::Any << TriggerSpeed::Slow << true;
    QTest::addRow("fast, any") << TriggerSpeed::Fast << TriggerSpeed::Any << false;
    QTest::addRow("fast, fast") << TriggerSpeed::Fast << TriggerSpeed::Fast << true;
    QTest::addRow("fast, slow") << TriggerSpeed::Fast << TriggerSpeed::Slow << false;
    QTest::addRow("slow, any") << TriggerSpeed::Slow << TriggerSpeed::Any << false;
    QTest::addRow("slow, fast") << TriggerSpeed::Slow << TriggerSpeed::Fast << false;
    QTest::addRow("slow, slow") << TriggerSpeed::Slow << TriggerSpeed::Slow << true;
}

void TestMotionTrigger::canUpdate()
{
    QFETCH(TriggerSpeed, triggerSpeed);
    QFETCH(TriggerSpeed, eventSpeed);
    QFETCH(bool, result);

    m_motionTrigger->setSpeed(triggerSpeed);
    m_event.setSpeed(eventSpeed);

    QCOMPARE(m_motionTrigger->canUpdate(&m_event), result);
}

void TestMotionTrigger::hasSpeed_data()
{
    QTest::addColumn<TriggerSpeed>("speed");
    QTest::addColumn<bool>("result");

    QTest::addRow("any, any") << TriggerSpeed::Any << false;
    QTest::addRow("fast") << TriggerSpeed::Fast << true;
    QTest::addRow("slow") << TriggerSpeed::Slow << true;
}

void TestMotionTrigger::hasSpeed()
{
    QFETCH(TriggerSpeed, speed);
    QFETCH(bool, result);

    m_motionTrigger->setSpeed(speed);

    QCOMPARE(m_motionTrigger->hasSpeed(), result);
}

}

QTEST_MAIN(libinputactions::TestMotionTrigger)
#include "test_motiontrigger.moc"