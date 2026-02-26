#include "TestMotionTriggerHandler.h"
#include "mocks/MockMotionTriggerHandler.h"
#include <libinputactions/handlers/TriggerHandler.h>
#include <libinputactions/helpers/Math.h>
#include <libinputactions/triggers/SwipeTrigger.h>

using namespace ::testing;

namespace InputActions
{

void TestMotionTriggerHandler::init()
{
    m_handler = std::make_unique<MockMotionTriggerHandler>();
    m_mouse = std::make_unique<InputDevice>(InputDeviceType::Mouse);
    m_mouse->properties().setMotionThreshold(10);
}

void TestMotionTriggerHandler::handleMotion_swipe__calculatesAnglesCorrectly_data()
{
    QTest::addColumn<QPointF>("delta");
    QTest::addColumn<qreal>("expectedAngles");

    QTest::addRow("[10, 0] => 0") << QPointF(10, 0) << 0.0;
    QTest::addRow("[10, -10] => 45") << QPointF(10, -10) << 45.0;
    QTest::addRow("[0, -10] => 90") << QPointF(0, -10) << 90.0;
    QTest::addRow("[-10, -10] => 90") << QPointF(-10, -10) << 135.0;
    QTest::addRow("[-10, 0] => 180") << QPointF(-10, 0) << 180.0;
    QTest::addRow("[-10, 10] => 225") << QPointF(-10, 10) << 225.0;
    QTest::addRow("[0, 10] => 270") << QPointF(0, 10) << 270.0;
    QTest::addRow("[10, 10] => 315") << QPointF(10, 10) << 315.0;
}

void TestMotionTriggerHandler::handleMotion_swipe__calculatesAnglesCorrectly()
{
    QFETCH(QPointF, delta);
    QFETCH(qreal, expectedAngles);

    EXPECT_CALL(*m_handler,
                updateTriggers(Contains(Pair(TriggerType::Swipe,
                                             WhenDynamicCastTo<const SwipeTriggerUpdateEvent *>(Pointee(AllOf(Property(&SwipeTriggerUpdateEvent::angle,
                                                                                                                       expectedAngles),
                                                                                                              Property(&SwipeTriggerUpdateEvent::averageAngle,
                                                                                                                       expectedAngles))))))))
        .Times(1);

    ON_CALL(*m_handler, hasActiveTriggers(static_cast<TriggerTypes>(TriggerType::SinglePointMotion))).WillByDefault(Return(true));
    ON_CALL(*m_handler, hasActiveTriggers(static_cast<TriggerTypes>(TriggerType::Swipe))).WillByDefault(Return(true));

    m_handler->handleMotion(m_mouse.get(), PointDelta(delta));

    QVERIFY(Mock::VerifyAndClearExpectations(m_handler.get()));
}

void TestMotionTriggerHandler::handleMotion_swipe__calculatesAverageAngleCorrectly()
{
    ON_CALL(*m_handler, hasActiveTriggers(static_cast<TriggerTypes>(TriggerType::SinglePointMotion))).WillByDefault(Return(true));
    ON_CALL(*m_handler, hasActiveTriggers(static_cast<TriggerTypes>(TriggerType::Swipe))).WillByDefault(Return(true));

    EXPECT_CALL(*m_handler,
                updateTriggers(Contains(Pair(TriggerType::Swipe,
                                             WhenDynamicCastTo<const SwipeTriggerUpdateEvent *>(Pointee(Property(&SwipeTriggerUpdateEvent::averageAngle,
                                                                                                                 0)))))))
        .Times(1);
    m_handler->handleMotion(m_mouse.get(), PointDelta({10, 0}));
    QVERIFY(Mock::VerifyAndClearExpectations(m_handler.get()));

    EXPECT_CALL(*m_handler,
                updateTriggers(Contains(Pair(TriggerType::Swipe,
                                             WhenDynamicCastTo<const SwipeTriggerUpdateEvent *>(Pointee(Property(&SwipeTriggerUpdateEvent::averageAngle,
                                                                                                                 0)))))))
        .Times(1);
    m_handler->handleMotion(m_mouse.get(), PointDelta({8, 0}));
    QVERIFY(Mock::VerifyAndClearExpectations(m_handler.get()));

    EXPECT_CALL(*m_handler,
                updateTriggers(Contains(Pair(TriggerType::Swipe,
                                             WhenDynamicCastTo<const SwipeTriggerUpdateEvent *>(Pointee(Property(&SwipeTriggerUpdateEvent::averageAngle,
                                                                                                                 Math::atan2deg360({4, 4}))))))))
        .Times(1);
    m_handler->handleMotion(m_mouse.get(), PointDelta({0, -8}));
    QVERIFY(Mock::VerifyAndClearExpectations(m_handler.get()));

    EXPECT_CALL(*m_handler,
                updateTriggers(Contains(Pair(TriggerType::Swipe,
                                             WhenDynamicCastTo<const SwipeTriggerUpdateEvent *>(Pointee(Property(&SwipeTriggerUpdateEvent::averageAngle,
                                                                                                                 Math::atan2deg360({0, 8}))))))))
        .Times(1);
    m_handler->handleMotion(m_mouse.get(), PointDelta({0, -8}));
    QVERIFY(Mock::VerifyAndClearExpectations(m_handler.get()));
}

void TestMotionTriggerHandler::handleMotion_swipe__motionBeforeThresholdIsTakenIntoAccountWhenCalculatingAverageAngle()
{
    ON_CALL(*m_handler, hasActiveTriggers(static_cast<TriggerTypes>(TriggerType::SinglePointMotion))).WillByDefault(Return(true));
    ON_CALL(*m_handler, hasActiveTriggers(static_cast<TriggerTypes>(TriggerType::Swipe))).WillByDefault(Return(true));

    m_handler->handleMotion(m_mouse.get(), PointDelta({3, 0}));
    m_handler->handleMotion(m_mouse.get(), PointDelta({3, 0}));

    EXPECT_CALL(*m_handler,
                updateTriggers(Contains(Pair(TriggerType::Swipe,
                                             WhenDynamicCastTo<const SwipeTriggerUpdateEvent *>(Pointee(Property(&SwipeTriggerUpdateEvent::averageAngle,
                                                                                                                 Math::atan2deg360({2, 3}))))))))
        .Times(1);

    m_handler->handleMotion(m_mouse.get(), PointDelta({0, -9}));

    QVERIFY(Mock::VerifyAndClearExpectations(m_handler.get()));
}

void TestMotionTriggerHandler::handleMotion_swipe_updatedOnceThenCancelled__activatesSwipeTriggers()
{
    ON_CALL(*m_handler, hasActiveTriggers(static_cast<TriggerTypes>(TriggerType::SinglePointMotion))).WillByDefault(Return(true));
    ON_CALL(*m_handler, hasActiveTriggers(static_cast<TriggerTypes>(TriggerType::Swipe))).WillByDefault(Return(true));

    EXPECT_CALL(*m_handler, updateTriggers(_))
        .Times(3)
        .WillOnce(Return(TriggerManagementOperationResult{
            .success = true,
        }))
        .WillRepeatedly(Return(TriggerManagementOperationResult{
            .success = false,
        }));

    m_handler->handleMotion(m_mouse.get(), PointDelta({10, 0}));

    EXPECT_CALL(*m_handler, activateTriggers(static_cast<TriggerTypes>(TriggerType::Swipe), _)).Times(1);
    m_handler->handleMotion(m_mouse.get(), PointDelta({10, 0}));
    QVERIFY(Mock::VerifyAndClearExpectations(m_handler.get()));
}

void TestMotionTriggerHandler::handleMotion_swipe_cancelledWithoutUpdate__doesNotActivateSwipeTriggers()
{
    ON_CALL(*m_handler, hasActiveTriggers(static_cast<TriggerTypes>(TriggerType::SinglePointMotion))).WillByDefault(Return(true));
    ON_CALL(*m_handler, hasActiveTriggers(static_cast<TriggerTypes>(TriggerType::Swipe))).WillByDefault(Return(true));

    EXPECT_CALL(*m_handler, updateTriggers(_))
        .Times(1)
        .WillRepeatedly(Return(TriggerManagementOperationResult{
            .success = false,
        }));
    EXPECT_CALL(*m_handler, activateTriggers(_, _)).Times(0);

    m_handler->handleMotion(m_mouse.get(), PointDelta({10, 0}));

    QVERIFY(Mock::VerifyAndClearExpectations(m_handler.get()));
}

}

QTEST_MAIN(InputActions::TestMotionTriggerHandler)