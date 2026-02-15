#include "TestTouchpadTriggerHandler.h"
#include "Test.h"
#include <libinputactions/input/backends/InputBackend.h>
#include <QSignalSpy>
#include <libinputactions/conditions/VariableCondition.h>
#include <libinputactions/handlers/TouchpadTriggerHandler.h>
#include <libinputactions/input/events.h>
#include <libinputactions/input/devices/InputDeviceProperties.h>
#include <libinputactions/input/devices/InputDeviceState.h>
#include <libinputactions/input/MouseButton.h>
#include <libinputactions/variables/VariableManager.h>
#include <linux/input-event-codes.h>
#include <ranges>

using namespace ::testing;

namespace InputActions
{

void TestTouchpadTriggerHandler::init()
{
    g_inputBackend = std::make_unique<InputBackend>();
    g_inputBackend->setTouchpadTriggerHandlerFactory([this](InputDevice *device) {
        auto handler = std::make_unique<MockTouchpadTriggerHandler>(device);
        m_handler = handler.get();
        return handler;
    });
    g_inputBackend->initialize();

    m_touchpad = std::make_unique<InputDevice>(InputDeviceType::Touchpad);
    m_touchpad->properties().setSize({100, 100});
    g_inputBackend->addDevice(m_touchpad.get());

    m_activatingTriggerSpy = std::make_unique<QSignalSpy>(m_handler, &TriggerHandler::activatingTrigger);
    m_activatingTriggersSpy = std::make_unique<QSignalSpy>(m_handler, &TriggerHandler::activatingTriggers);
    m_cancellingTriggersSpy = std::make_unique<QSignalSpy>(m_handler, &TriggerHandler::cancellingTriggers);
    m_endingTriggersSpy = std::make_unique<QSignalSpy>(m_handler, &TriggerHandler::endingTriggers);
}

void TestTouchpadTriggerHandler::click_withoutLibinputButton()
{
    m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Click));

    g_inputBackend->handleEvent(TouchpadClickEvent(m_touchpad.get(), true));
    QCOMPARE(m_activatingTriggersSpy->count(), 1);
    QCOMPARE(m_activatingTriggersSpy->at(0).at(0).value<TriggerTypes>(), TriggerType::Click);

    g_inputBackend->handleEvent(TouchpadClickEvent(m_touchpad.get(), false));
    QCOMPARE(m_endingTriggersSpy->count(), 1);
    QCOMPARE(m_endingTriggersSpy->at(0).at(0).value<TriggerTypes>(), TriggerType::Click);

    QCOMPARE(m_handler->m_state, TouchpadTriggerHandler::State::None);
}

void TestTouchpadTriggerHandler::click_withLibinputButton_data()
{
    QTest::addColumn<int>("button");

    QTest::addRow("left") << BTN_LEFT;
    QTest::addRow("right") << BTN_RIGHT;
    QTest::addRow("middle") << BTN_MIDDLE;
}

void TestTouchpadTriggerHandler::click_withLibinputButton()
{
    QFETCH(int, button);

    m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Click));

    g_inputBackend->handleEvent(TouchpadClickEvent(m_touchpad.get(), true));
    QCOMPARE(g_inputBackend->handleEvent(PointerButtonEvent(m_touchpad.get(), button, true)), true);
    QCOMPARE(m_activatingTriggersSpy->count(), 1);
    QCOMPARE(m_activatingTriggersSpy->at(0).at(0).value<TriggerTypes>(), TriggerType::Click);

    g_inputBackend->handleEvent(TouchpadClickEvent(m_touchpad.get(), false));
    QCOMPARE(g_inputBackend->handleEvent(PointerButtonEvent(m_touchpad.get(), button, false)), true);
    QCOMPARE(m_endingTriggersSpy->count(), 1);
    QCOMPARE(m_endingTriggersSpy->at(0).at(0).value<TriggerTypes>(), TriggerType::Click);

    QCOMPARE(m_handler->m_state, TouchpadTriggerHandler::State::None);
}

void TestTouchpadTriggerHandler::press1_notDelayedOrBlocked()
{
    m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Press));

    QCOMPARE(g_inputBackend->handleEvent(TouchpadGestureLifecyclePhaseEvent(m_touchpad.get(), TouchpadGestureLifecyclePhase::Begin, TriggerType::Press, 1)), false);

    QCOMPARE(m_activatingTriggersSpy->count(), 1);
    QCOMPARE(m_activatingTriggersSpy->at(0).at(0).value<TriggerTypes>(), TriggerType::Press);

    QCOMPARE(g_inputBackend->handleEvent(TouchpadGestureLifecyclePhaseEvent(m_touchpad.get(), TouchpadGestureLifecyclePhase::End, TriggerType::Press)), false);

    QCOMPARE(m_handler->m_state, TouchpadTriggerHandler::State::None);
}

void TestTouchpadTriggerHandler::press1_hasClickTrigger_delayed()
{
    m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Press));
    m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Click));

    g_inputBackend->handleEvent(TouchpadGestureLifecyclePhaseEvent(m_touchpad.get(), TouchpadGestureLifecyclePhase::Begin, TriggerType::Press));
    QCOMPARE(m_activatingTriggersSpy->count(), 0);

    QTest::qWait(500);

    QCOMPARE(m_activatingTriggersSpy->count(), 1);
    QCOMPARE(m_activatingTriggersSpy->at(0).at(0).value<TriggerTypes>(), TriggerType::Press);

    QCOMPARE(m_handler->m_state, TouchpadTriggerHandler::State::None);
}

void TestTouchpadTriggerHandler::press1_hasTapTrigger_delayed()
{
    m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Press));
    m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Tap));

    g_inputBackend->handleEvent(TouchpadGestureLifecyclePhaseEvent(m_touchpad.get(), TouchpadGestureLifecyclePhase::Begin, TriggerType::Press));
    QCOMPARE(m_activatingTriggersSpy->count(), 0);

    QTest::qWait(500);

    QCOMPARE(m_activatingTriggersSpy->count(), 1);
    QCOMPARE(m_activatingTriggersSpy->at(0).at(0).value<TriggerTypes>(), TriggerType::Press);

    QCOMPARE(m_handler->m_state, TouchpadTriggerHandler::State::None);
}

void TestTouchpadTriggerHandler::press1_clickedDuringPress_pressCancelledAndClickActivated()
{
    m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Press));
    m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Click));

    g_inputBackend->handleEvent(TouchpadGestureLifecyclePhaseEvent(m_touchpad.get(), TouchpadGestureLifecyclePhase::Begin, TriggerType::Press));
    QTest::qWait(500);
    QCOMPARE(m_activatingTriggersSpy->count(), 1);
    QCOMPARE(m_activatingTriggersSpy->at(0).at(0).value<TriggerTypes>(), TriggerType::Press);

    g_inputBackend->handleEvent(TouchpadClickEvent(m_touchpad.get(), true));
    g_inputBackend->handleEvent(TouchpadGestureLifecyclePhaseEvent(m_touchpad.get(), TouchpadGestureLifecyclePhase::End, TriggerType::Press)); // libinput

    QCOMPARE(m_cancellingTriggersSpy->count(), 1);
    QCOMPARE(m_cancellingTriggersSpy->at(0).at(0).value<TriggerTypes>(), TriggerType::Press);
    QCOMPARE(m_endingTriggersSpy->count(), 0);
    QCOMPARE(m_activatingTriggersSpy->count(), 2);
    QCOMPARE(m_activatingTriggersSpy->at(1).at(0).value<TriggerTypes>(), TriggerType::Click);

    g_inputBackend->handleEvent(TouchpadClickEvent(m_touchpad.get(), false));

    QCOMPARE(m_handler->m_state, TouchpadTriggerHandler::State::None);
}

void TestTouchpadTriggerHandler::press2_notDelayedOrBlocked()
{
    m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Press));

    QCOMPARE(g_inputBackend->handleEvent(TouchpadGestureLifecyclePhaseEvent(m_touchpad.get(), TouchpadGestureLifecyclePhase::Begin, TriggerType::Press, 2)), false);

    QCOMPARE(m_activatingTriggersSpy->count(), 1);
    QCOMPARE(m_activatingTriggersSpy->at(0).at(0).value<TriggerTypes>(), TriggerType::Press);

    QCOMPARE(g_inputBackend->handleEvent(TouchpadGestureLifecyclePhaseEvent(m_touchpad.get(), TouchpadGestureLifecyclePhase::End, TriggerType::Press)), false);

    QCOMPARE(m_handler->m_state, TouchpadTriggerHandler::State::None);
}

void TestTouchpadTriggerHandler::press3_blocked()
{
    m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Press));

    QCOMPARE(g_inputBackend->handleEvent(TouchpadGestureLifecyclePhaseEvent(m_touchpad.get(), TouchpadGestureLifecyclePhase::Begin, TriggerType::Press, 3)), true);
    QCOMPARE(g_inputBackend->handleEvent(TouchpadGestureLifecyclePhaseEvent(m_touchpad.get(), TouchpadGestureLifecyclePhase::End, TriggerType::Press)), true);

    QCOMPARE(m_handler->m_state, TouchpadTriggerHandler::State::None);
}

void TestTouchpadTriggerHandler::swipe1()
{
    auto trigger = std::make_unique<Trigger>(TriggerType::Swipe);
    trigger->setActivationCondition(std::make_shared<VariableCondition>("fingers", InputActions::Value<qreal>(1), ComparisonOperator::EqualTo));
    m_handler->addTrigger(std::move(trigger));

    addPoints(1);
    movePoints({0.05, 0});
    QCOMPARE(g_inputBackend->handleEvent(MotionEvent(m_touchpad.get(), InputEventType::PointerMotion, {{10, 0}})), true);
    QCOMPARE(m_activatingTriggerSpy->count(), 1);

    removePoints();
    QCOMPARE(m_endingTriggersSpy->count(), 1);
    QVERIFY(m_endingTriggersSpy->at(0).at(0).value<TriggerTypes>() & TriggerType::SinglePointMotion);

    QCOMPARE(m_handler->m_state, TouchpadTriggerHandler::State::None);
}

void TestTouchpadTriggerHandler::swipe2()
{
    auto trigger = std::make_unique<Trigger>(TriggerType::Swipe);
    trigger->setActivationCondition(std::make_shared<VariableCondition>("fingers", InputActions::Value<qreal>(2), ComparisonOperator::EqualTo));
    m_handler->addTrigger(std::move(trigger));

    addPoints(2);
    movePoints({0.05, 0});
    movePoints({0.05, 0});
    movePoints({0.05, 0});
    QCOMPARE(g_inputBackend->handleEvent(MotionEvent(m_touchpad.get(), InputEventType::PointerAxis, {{10, 0}})), true);
    QCOMPARE(m_activatingTriggerSpy->count(), 1);

    QCOMPARE(g_inputBackend->handleEvent(MotionEvent(m_touchpad.get(), InputEventType::PointerAxis, {{0, 0}})), false);
    QCOMPARE(m_endingTriggersSpy->count(), 1);
    QCOMPARE(m_endingTriggersSpy->at(0).at(0).value<TriggerTypes>(), TriggerType::SinglePointMotion);

    QCOMPARE(m_handler->m_state, TouchpadTriggerHandler::State::None);
}

void TestTouchpadTriggerHandler::tap1()
{
    m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Tap));

    addPoint();
    removePoints();

    // should not activate without libinput click
    QCOMPARE(m_activatingTriggersSpy->count(), 0);

    // libinput click
    QCOMPARE(g_inputBackend->handleEvent(PointerButtonEvent(m_touchpad.get(), BTN_LEFT, true)), true);
    QCOMPARE(m_activatingTriggersSpy->count(), 1);
    QCOMPARE(m_activatingTriggersSpy->at(0).at(0).value<TriggerTypes>(), TriggerType::Tap);
    QCOMPARE(g_inputBackend->handleEvent(PointerButtonEvent(m_touchpad.get(), BTN_LEFT, false)), true);
    QCOMPARE(m_endingTriggersSpy->count(), 1);
    QCOMPARE(m_endingTriggersSpy->at(0).at(0).value<TriggerTypes>(), TriggerType::Tap);

    QCOMPARE(g_inputBackend->handleEvent(PointerButtonEvent(m_touchpad.get(), BTN_LEFT, true)), false);
    QCOMPARE(g_inputBackend->handleEvent(PointerButtonEvent(m_touchpad.get(), BTN_LEFT, false)), false);

    QCOMPARE(m_handler->m_state, TouchpadTriggerHandler::State::None);
}

void TestTouchpadTriggerHandler::tap1_noPointerButtonEvent_stateReset()
{
    m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Tap));

    addPoint();
    removePoints();

    QCOMPARE(m_handler->m_state, TouchpadTriggerHandler::State::LibinputTapBegin);
    QTest::qWait(500);
    QCOMPARE(m_handler->m_state, TouchpadTriggerHandler::State::None);
}

void TestTouchpadTriggerHandler::tap1_tappedAgainBeforeLibinputButtonReleased()
{
    m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Tap));

    addPoint();
    removePoints();

    QCOMPARE(g_inputBackend->handleEvent(PointerButtonEvent(m_touchpad.get(), BTN_LEFT, true)), true);
    QCOMPARE(m_activatingTriggersSpy->count(), 1);
    QCOMPARE(m_activatingTriggersSpy->at(0).at(0).value<TriggerTypes>(), TriggerType::Tap);

    addPoint();
    removePoints();

    QCOMPARE(g_inputBackend->handleEvent(PointerButtonEvent(m_touchpad.get(), BTN_LEFT, false)), true);
    QCOMPARE(m_endingTriggersSpy->count(), 1);
    QCOMPARE(m_endingTriggersSpy->at(0).at(0).value<TriggerTypes>(), TriggerType::Tap);

    QCOMPARE(g_inputBackend->handleEvent(PointerButtonEvent(m_touchpad.get(), BTN_LEFT, true)), true);
    QCOMPARE(m_activatingTriggersSpy->count(), 2);
    QCOMPARE(m_activatingTriggersSpy->at(1).at(0).value<TriggerTypes>(), TriggerType::Tap);

    QCOMPARE(g_inputBackend->handleEvent(PointerButtonEvent(m_touchpad.get(), BTN_LEFT, false)), true);
    QCOMPARE(m_endingTriggersSpy->count(), 2);
    QCOMPARE(m_endingTriggersSpy->at(1).at(0).value<TriggerTypes>(), TriggerType::Tap);

    QCOMPARE(m_handler->m_state, TouchpadTriggerHandler::State::None);
}

void TestTouchpadTriggerHandler::tap2_variablesSetDuringActivation()
{
    m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Tap));

    const QPointF first(10, 10);
    const QPointF second(20, 20);
    addPoint(first);
    addPoint(second);

    const auto finger1Position = g_variableManager->getVariable<QPointF>("finger_1_position_percentage");
    const auto finger2Position = g_variableManager->getVariable<QPointF>("finger_2_position_percentage");
    QCOMPARE(finger1Position->get(), QPointF(0.1, 0.1));
    QCOMPARE(finger2Position->get(), QPointF(0.2, 0.2));

    removePoints(1);
    QCOMPARE(finger1Position->get(), QPointF(0.1, 0.1));
    QCOMPARE(finger2Position->get(), QPointF(0.2, 0.2));

    removePoints(1);
    QCOMPARE(finger1Position->get(), QPointF(0.1, 0.1));
    QCOMPARE(finger2Position->get(), QPointF(0.2, 0.2));

    g_inputBackend->handleEvent(PointerButtonEvent(m_touchpad.get(), BTN_LEFT, true));
    g_inputBackend->handleEvent(PointerButtonEvent(m_touchpad.get(), BTN_LEFT, false));
    QVERIFY(!finger1Position->get().has_value());
    QVERIFY(!finger2Position->get().has_value());

    QCOMPARE(m_handler->m_state, TouchpadTriggerHandler::State::None);
}

void TestTouchpadTriggerHandler::tap4()
{
    m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Tap));

    addPoints(4);
    removePoints();

    QCOMPARE(m_activatingTriggersSpy->count(), 1);
    QCOMPARE(m_activatingTriggersSpy->at(0).at(0).value<TriggerTypes>(), TriggerType::Tap);
    QCOMPARE(m_endingTriggersSpy->count(), 1);
    QCOMPARE(m_endingTriggersSpy->at(0).at(0).value<TriggerTypes>(), TriggerType::Tap);

    QCOMPARE(m_handler->m_state, TouchpadTriggerHandler::State::None);
}

void TestTouchpadTriggerHandler::tap4_moved()
{
    m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Tap));

    addPoints(4);
    movePoints({10, 10});
    removePoints();

    QCOMPARE(m_activatingTriggersSpy->count(), 0);

    QCOMPARE(m_handler->m_state, TouchpadTriggerHandler::State::None);
}

void TestTouchpadTriggerHandler::tap4_slow()
{
    m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Tap));

    addPoints(4);
    QTest::qWait(500);
    removePoints();

    QCOMPARE(m_activatingTriggersSpy->count(), 0);

    QCOMPARE(m_handler->m_state, TouchpadTriggerHandler::State::None);
}

void TestTouchpadTriggerHandler::tap4_clicked()
{
    m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Click));
    m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Tap));

    addPoints(4);
    g_inputBackend->handleEvent(TouchpadClickEvent(m_touchpad.get(), true));
    g_inputBackend->handleEvent(TouchpadClickEvent(m_touchpad.get(), false));
    removePoints();

    QCOMPARE(m_activatingTriggersSpy->count(), 1);
    QCOMPARE(m_activatingTriggersSpy->at(0).at(0).value<TriggerTypes>(), TriggerType::Click);

    QCOMPARE(m_handler->m_state, TouchpadTriggerHandler::State::None);
}

void TestTouchpadTriggerHandler::tap_fingerCount_data()
{
    QTest::addColumn<int>("triggerFingers");
    QTest::addColumn<int>("fingers");
    QTest::addColumn<int>("button");
    QTest::addColumn<bool>("lmrTapButtonMap");
    QTest::addColumn<bool>("activated");

    QTest::addRow("1") << 1 << 1 << BTN_LEFT << false << true;
    QTest::addRow("2") << 2 << 2 << BTN_RIGHT << false << true;
    QTest::addRow("3") << 3 << 3 << BTN_MIDDLE << false << true;
    QTest::addRow("4") << 4 << 4 << 0 << false << true;
    QTest::addRow("5") << 5 << 5 << 0 << false << true;
    QTest::addRow("wrong") << 2 << 1 << BTN_LEFT << false << false;
    QTest::addRow("1 lmr") << 1 << 1 << BTN_LEFT << true << true;
    QTest::addRow("2 lmr") << 2 << 2 << BTN_MIDDLE << true << true;
    QTest::addRow("3 lmr") << 3 << 3 << BTN_RIGHT << true << true;
    QTest::addRow("4 lmr") << 4 << 4 << 0 << true << true;
    QTest::addRow("5 lmr") << 5 << 5 << 0 << true << true;
}

void TestTouchpadTriggerHandler::tap_fingerCount()
{
    QFETCH(int, triggerFingers);
    QFETCH(int, fingers);
    QFETCH(int, button);
    QFETCH(bool, lmrTapButtonMap);
    QFETCH(bool, activated);

    auto trigger = std::make_unique<Trigger>(TriggerType::Tap);
    trigger->setActivationCondition(std::make_shared<VariableCondition>("fingers", InputActions::Value<qreal>(triggerFingers), ComparisonOperator::EqualTo));
    m_handler->addTrigger(std::move(trigger));
    m_touchpad->properties().setTouchpadLmrTapButtonMap(lmrTapButtonMap);

    addPoints(fingers);
    removePoints();
    if (button) {
        g_inputBackend->handleEvent(PointerButtonEvent(m_touchpad.get(), button, true));
        g_inputBackend->handleEvent(PointerButtonEvent(m_touchpad.get(), button, false));
    }

    QCOMPARE(m_activatingTriggerSpy->count(), activated);

    QCOMPARE(m_handler->m_state, TouchpadTriggerHandler::State::None);
}

void TestTouchpadTriggerHandler::pointerAxis_oneAxisPerEvent_firstEventPassedThrough()
{
    EXPECT_CALL(*m_handler, handleMotion(m_touchpad.get(), PointDelta({1, 1}))).Times(1);

    m_handler->pointerAxis(MotionEvent(m_touchpad.get(), InputEventType::PointerAxis, {{1, 1}}, true));

    QVERIFY(Mock::VerifyAndClearExpectations(m_handler));
}

void TestTouchpadTriggerHandler::pointerAxis_oneAxisPerEvent_eventsBlocked()
{
    EXPECT_CALL(*m_handler, handleMotion(m_touchpad.get(), _)).WillRepeatedly(Return(true));

    // First is passed through
    QVERIFY(m_handler->pointerAxis(MotionEvent(m_touchpad.get(), InputEventType::PointerAxis, {{1, 1}}, true)));

    QVERIFY(m_handler->pointerAxis(MotionEvent(m_touchpad.get(), InputEventType::PointerAxis, {{1, 0}}, true)));
    QVERIFY(m_handler->pointerAxis(MotionEvent(m_touchpad.get(), InputEventType::PointerAxis, {{0, 1}}, true)));
}

void TestTouchpadTriggerHandler::pointerAxis_oneAxisPerEvent_eventsNotBlocked()
{
    EXPECT_CALL(*m_handler, handleMotion(m_touchpad.get(), _)).WillRepeatedly(Return(false));

    // First is passed through
    QVERIFY(!m_handler->pointerAxis(MotionEvent(m_touchpad.get(), InputEventType::PointerAxis, {{1, 1}}, true)));

    QVERIFY(!m_handler->pointerAxis(MotionEvent(m_touchpad.get(), InputEventType::PointerAxis, {{1, 0}}, true)));
    QVERIFY(!m_handler->pointerAxis(MotionEvent(m_touchpad.get(), InputEventType::PointerAxis, {{0, 1}}, true)));
}

void TestTouchpadTriggerHandler::pointerAxis_oneAxisPerEvent_eventBlockingStops()
{
    auto block = true;
    EXPECT_CALL(*m_handler, handleMotion(m_touchpad.get(), _)).WillRepeatedly([&block]() {
        return block;
    });

    // First is passed through
    QVERIFY(m_handler->pointerAxis(MotionEvent(m_touchpad.get(), InputEventType::PointerAxis, {{1, 1}}, true)));

    QVERIFY(m_handler->pointerAxis(MotionEvent(m_touchpad.get(), InputEventType::PointerAxis, {{1, 0}}, true)));
    block = false;
    QVERIFY(!m_handler->pointerAxis(MotionEvent(m_touchpad.get(), InputEventType::PointerAxis, {{0, 1}}, true)));

    QVERIFY(!m_handler->pointerAxis(MotionEvent(m_touchpad.get(), InputEventType::PointerAxis, {{1, 0}}, true)));
    QVERIFY(!m_handler->pointerAxis(MotionEvent(m_touchpad.get(), InputEventType::PointerAxis, {{0, 1}}, true)));
}

void TestTouchpadTriggerHandler::pointerAxis_oneAxisPerEvent_differentAxisEventsMerged()
{
    // First is passed through
    m_handler->pointerAxis(MotionEvent(m_touchpad.get(), InputEventType::PointerAxis, {{1, 1}}, true));

    EXPECT_CALL(*m_handler, handleMotion(m_touchpad.get(), PointDelta({1, 1}))).Times(1);

    m_handler->pointerAxis(MotionEvent(m_touchpad.get(), InputEventType::PointerAxis, {{1, 0}}, true));
    m_handler->pointerAxis(MotionEvent(m_touchpad.get(), InputEventType::PointerAxis, {{0, 1}}, true));

    QVERIFY(Mock::VerifyAndClearExpectations(m_handler));
}

void TestTouchpadTriggerHandler::pointerAxis_oneAxisPerEvent_sameAxisEventsNotMerged()
{
    // First is passed through
    m_handler->pointerAxis(MotionEvent(m_touchpad.get(), InputEventType::PointerAxis, {{1, 1}}, true));

    EXPECT_CALL(*m_handler, handleMotion(m_touchpad.get(), PointDelta({1, 0}))).Times(2);
    m_handler->pointerAxis(MotionEvent(m_touchpad.get(), InputEventType::PointerAxis, {{1, 0}}, true));
    m_handler->pointerAxis(MotionEvent(m_touchpad.get(), InputEventType::PointerAxis, {{1, 0}}, true));
    QVERIFY(Mock::VerifyAndClearExpectations(m_handler));

    EXPECT_CALL(*m_handler, handleMotion(m_touchpad.get(), PointDelta({0, 1}))).Times(2);
    m_handler->pointerAxis(MotionEvent(m_touchpad.get(), InputEventType::PointerAxis, {{0, 1}}, true));
    m_handler->pointerAxis(MotionEvent(m_touchpad.get(), InputEventType::PointerAxis, {{0, 1}}, true));
    QVERIFY(Mock::VerifyAndClearExpectations(m_handler));
}

void TestTouchpadTriggerHandler::pointerAxis_notOneAxisPerEvent_notMerged()
{
    // First would be passed through
    m_handler->pointerAxis(MotionEvent(m_touchpad.get(), InputEventType::PointerAxis, {{1, 1}}, false));

    EXPECT_CALL(*m_handler, handleMotion(m_touchpad.get(), PointDelta({1, 0}))).Times(1);
    m_handler->pointerAxis(MotionEvent(m_touchpad.get(), InputEventType::PointerAxis, {{1, 0}}, false));
    QVERIFY(Mock::VerifyAndClearExpectations(m_handler));
}

void TestTouchpadTriggerHandler::addPoint(const QPointF &position)
{
    m_touchId++;
    g_inputBackend->handleEvent(TouchDownEvent(m_touchpad.get(), m_touchId, position, position));
}

void TestTouchpadTriggerHandler::addPoints(uint8_t count, const QPointF &position)
{
    for (auto i = 0; i < count; i++) {
        addPoint(position);
    }
}

void TestTouchpadTriggerHandler::movePoints(const QPointF &delta)
{
    for (auto *point : m_touchpad->physicalState().validTouchPoints()) {
        g_inputBackend->handleEvent(TouchMotionEvent(m_touchpad.get(), point->id, point->position + delta, point->rawPosition + delta));
    }
}

void TestTouchpadTriggerHandler::removePoints(int16_t count)
{
    uint8_t removed{};
    for (auto *point : std::ranges::reverse_view(m_touchpad->physicalState().validTouchPoints())) {
        g_inputBackend->handleEvent(TouchUpEvent(m_touchpad.get(), point->id));
        if (++removed == count) {
            break;
        }
    }
}

};

QTEST_MAIN(InputActions::TestTouchpadTriggerHandler)