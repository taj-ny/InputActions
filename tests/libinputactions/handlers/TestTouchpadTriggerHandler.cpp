#include "TestTouchpadTriggerHandler.h"
#include <QSignalSpy>
#include <QTest>
#include <libinputactions/conditions/VariableCondition.h>
#include <libinputactions/handlers/TouchpadTriggerHandler.h>
#include <libinputactions/variables/VariableManager.h>
#include <linux/input-event-codes.h>
#include <ranges>

namespace libinputactions
{

void TestTouchpadTriggerHandler::init()
{
    m_handler = std::make_unique<TouchpadTriggerHandler>();
    m_activatingTriggerSpy = std::make_unique<QSignalSpy>(m_handler.get(), &TriggerHandler::activatingTrigger);
    m_activatingTriggersSpy = std::make_unique<QSignalSpy>(m_handler.get(), &TriggerHandler::activatingTriggers);
    m_cancellingTriggersSpy = std::make_unique<QSignalSpy>(m_handler.get(), &TriggerHandler::cancellingTriggers);
    m_endingTriggersSpy = std::make_unique<QSignalSpy>(m_handler.get(), &TriggerHandler::endingTriggers);
    m_touchpad = std::make_unique<InputDevice>(InputDeviceType::Touchpad);
    m_touchpad->m_touchPoints = std::vector<TouchPoint>(5);
}

void TestTouchpadTriggerHandler::click_withoutLibinputButton()
{
    m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Click));

    m_handler->handleEvent(TouchpadClickEvent(m_touchpad.get(), true));
    QCOMPARE(m_activatingTriggersSpy->count(), 1);
    QCOMPARE(m_activatingTriggersSpy->at(0).at(0).value<TriggerTypes>(), TriggerType::Click);

    m_handler->handleEvent(TouchpadClickEvent(m_touchpad.get(), false));
    QCOMPARE(m_endingTriggersSpy->count(), 1);
    QCOMPARE(m_endingTriggersSpy->at(0).at(0).value<TriggerTypes>(), TriggerType::Click);
}

void TestTouchpadTriggerHandler::click_withLibinputButton_data()
{
    QTest::addColumn<Qt::MouseButton>("button");
    QTest::addColumn<int>("nativeButton");

    QTest::addRow("left") << Qt::MouseButton::LeftButton << BTN_LEFT;
    QTest::addRow("right") << Qt::MouseButton::RightButton << BTN_RIGHT;
    QTest::addRow("middle") << Qt::MouseButton::MiddleButton << BTN_MIDDLE;
}

void TestTouchpadTriggerHandler::click_withLibinputButton()
{
    QFETCH(Qt::MouseButton, button);
    QFETCH(int, nativeButton);

    m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Click));

    m_handler->handleEvent(TouchpadClickEvent(m_touchpad.get(), true));
    QCOMPARE(m_handler->handleEvent(PointerButtonEvent(m_touchpad.get(), button, nativeButton, true)), true);
    QCOMPARE(m_activatingTriggersSpy->count(), 1);
    QCOMPARE(m_activatingTriggersSpy->at(0).at(0).value<TriggerTypes>(), TriggerType::Click);

    m_handler->handleEvent(TouchpadClickEvent(m_touchpad.get(), false));
    QCOMPARE(m_handler->handleEvent(PointerButtonEvent(m_touchpad.get(), button, nativeButton, false)), true);
    QCOMPARE(m_endingTriggersSpy->count(), 1);
    QCOMPARE(m_endingTriggersSpy->at(0).at(0).value<TriggerTypes>(), TriggerType::Click);
}

void TestTouchpadTriggerHandler::press1_notDelayedOrBlocked()
{
    m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Press));

    QCOMPARE(m_handler->handleEvent(TouchpadGestureLifecyclePhaseEvent(m_touchpad.get(), TouchpadGestureLifecyclePhase::Begin, TriggerType::Press, 1)), false);

    QCOMPARE(m_activatingTriggersSpy->count(), 1);
    QCOMPARE(m_activatingTriggersSpy->at(0).at(0).value<TriggerTypes>(), TriggerType::Press);

    QCOMPARE(m_handler->handleEvent(TouchpadGestureLifecyclePhaseEvent(m_touchpad.get(), TouchpadGestureLifecyclePhase::End, TriggerType::Press)), false);
}

void TestTouchpadTriggerHandler::press1_hasClickTrigger_delayed()
{
    m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Press));
    m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Click));

    m_handler->handleEvent(TouchpadGestureLifecyclePhaseEvent(m_touchpad.get(), TouchpadGestureLifecyclePhase::Begin, TriggerType::Press));
    QCOMPARE(m_activatingTriggersSpy->count(), 0);

    QTest::qWait(500);

    QCOMPARE(m_activatingTriggersSpy->count(), 1);
    QCOMPARE(m_activatingTriggersSpy->at(0).at(0).value<TriggerTypes>(), TriggerType::Press);
}

void TestTouchpadTriggerHandler::press1_hasTapTrigger_delayed()
{
    m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Press));
    m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Tap));

    m_handler->handleEvent(TouchpadGestureLifecyclePhaseEvent(m_touchpad.get(), TouchpadGestureLifecyclePhase::Begin, TriggerType::Press));
    QCOMPARE(m_activatingTriggersSpy->count(), 0);

    QTest::qWait(500);

    QCOMPARE(m_activatingTriggersSpy->count(), 1);
    QCOMPARE(m_activatingTriggersSpy->at(0).at(0).value<TriggerTypes>(), TriggerType::Press);
}

void TestTouchpadTriggerHandler::press1_clickedDuringPress_pressCancelledAndClickActivated()
{
    m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Press));
    m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Click));

    m_handler->handleEvent(TouchpadGestureLifecyclePhaseEvent(m_touchpad.get(), TouchpadGestureLifecyclePhase::Begin, TriggerType::Press));
    QTest::qWait(500);
    QCOMPARE(m_activatingTriggersSpy->count(), 1);
    QCOMPARE(m_activatingTriggersSpy->at(0).at(0).value<TriggerTypes>(), TriggerType::Press);

    m_handler->handleEvent(TouchpadClickEvent(m_touchpad.get(), true));
    m_handler->handleEvent(TouchpadGestureLifecyclePhaseEvent(m_touchpad.get(), TouchpadGestureLifecyclePhase::End, TriggerType::Press)); // libinput

    QCOMPARE(m_cancellingTriggersSpy->count(), 1);
    QCOMPARE(m_cancellingTriggersSpy->at(0).at(0).value<TriggerTypes>(), TriggerType::Press);
    QCOMPARE(m_endingTriggersSpy->count(), 0);
    QCOMPARE(m_activatingTriggersSpy->count(), 2);
    QCOMPARE(m_activatingTriggersSpy->at(1).at(0).value<TriggerTypes>(), TriggerType::Click);
}

void TestTouchpadTriggerHandler::press2_notDelayedOrBlocked()
{
    m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Press));

    QCOMPARE(m_handler->handleEvent(TouchpadGestureLifecyclePhaseEvent(m_touchpad.get(), TouchpadGestureLifecyclePhase::Begin, TriggerType::Press, 2)), false);

    QCOMPARE(m_activatingTriggersSpy->count(), 1);
    QCOMPARE(m_activatingTriggersSpy->at(0).at(0).value<TriggerTypes>(), TriggerType::Press);

    QCOMPARE(m_handler->handleEvent(TouchpadGestureLifecyclePhaseEvent(m_touchpad.get(), TouchpadGestureLifecyclePhase::End, TriggerType::Press)), false);
}

void TestTouchpadTriggerHandler::press3_blocked()
{
    m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Press));

    QCOMPARE(m_handler->handleEvent(TouchpadGestureLifecyclePhaseEvent(m_touchpad.get(), TouchpadGestureLifecyclePhase::Begin, TriggerType::Press, 3)), true);
    QCOMPARE(m_handler->handleEvent(TouchpadGestureLifecyclePhaseEvent(m_touchpad.get(), TouchpadGestureLifecyclePhase::End, TriggerType::Press)), true);
}

void TestTouchpadTriggerHandler::swipe1()
{
    auto trigger = std::make_unique<Trigger>(TriggerType::Swipe);
    trigger->m_activationCondition = std::make_shared<VariableCondition>("fingers", static_cast<qreal>(1), ComparisonOperator::EqualTo);
    m_handler->addTrigger(std::move(trigger));

    addPoints(1);
    movePoints({0.05, 0});
    QCOMPARE(m_handler->handleEvent(MotionEvent(m_touchpad.get(), InputEventType::PointerMotion, {10, 0})), true);
    QCOMPARE(m_activatingTriggerSpy->count(), 1);

    removePoints();
    QCOMPARE(m_endingTriggersSpy->count(), 1);
    QVERIFY(m_endingTriggersSpy->at(0).at(0).value<TriggerTypes>() & TriggerType::StrokeSwipe);
}

void TestTouchpadTriggerHandler::swipe2()
{
    auto trigger = std::make_unique<Trigger>(TriggerType::Swipe);
    trigger->m_activationCondition = std::make_shared<VariableCondition>("fingers", static_cast<qreal>(2), ComparisonOperator::EqualTo);
    m_handler->addTrigger(std::move(trigger));

    addPoints(2);
    movePoints({0.05, 0});
    movePoints({0.05, 0});
    movePoints({0.05, 0});
    QCOMPARE(m_handler->handleScrollEvent(MotionEvent(m_touchpad.get(), InputEventType::PointerScroll, {10, 0})), true);
    QCOMPARE(m_activatingTriggerSpy->count(), 1);

    QCOMPARE(m_handler->handleScrollEvent(MotionEvent(m_touchpad.get(), InputEventType::PointerScroll, {0, 0})), false);
    QCOMPARE(m_endingTriggersSpy->count(), 1);
    QCOMPARE(m_endingTriggersSpy->at(0).at(0).value<TriggerTypes>(), TriggerType::StrokeSwipe);
}

void TestTouchpadTriggerHandler::tap1()
{
    m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Tap));

    addPoint();
    removePoints();

    // should not activate without libinput click
    QCOMPARE(m_activatingTriggersSpy->count(), 0);

    // libinput click
    QCOMPARE(m_handler->handleEvent(PointerButtonEvent(m_touchpad.get(), Qt::MouseButton::LeftButton, BTN_LEFT, true)), true);
    QCOMPARE(m_activatingTriggersSpy->count(), 1);
    QCOMPARE(m_activatingTriggersSpy->at(0).at(0).value<TriggerTypes>(), TriggerType::Tap);
    QCOMPARE(m_handler->handleEvent(PointerButtonEvent(m_touchpad.get(), Qt::MouseButton::LeftButton, BTN_LEFT, false)), true);
    QCOMPARE(m_endingTriggersSpy->count(), 1);
    QCOMPARE(m_endingTriggersSpy->at(0).at(0).value<TriggerTypes>(), TriggerType::Tap);

    QCOMPARE(m_handler->handleEvent(PointerButtonEvent(m_touchpad.get(), Qt::MouseButton::LeftButton, BTN_LEFT, true)), false);
    QCOMPARE(m_handler->handleEvent(PointerButtonEvent(m_touchpad.get(), Qt::MouseButton::LeftButton, BTN_LEFT, false)), false);
}

void TestTouchpadTriggerHandler::tap1_tappedAgainBeforeLibinputButtonReleased()
{
    m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Tap));

    addPoint();
    removePoints();

    QCOMPARE(m_handler->handleEvent(PointerButtonEvent(m_touchpad.get(), Qt::MouseButton::LeftButton, BTN_LEFT, true)), true);
    QCOMPARE(m_activatingTriggersSpy->count(), 1);
    QCOMPARE(m_activatingTriggersSpy->at(0).at(0).value<TriggerTypes>(), TriggerType::Tap);

    addPoint();
    removePoints();

    QCOMPARE(m_handler->handleEvent(PointerButtonEvent(m_touchpad.get(), Qt::MouseButton::LeftButton, BTN_LEFT, false)), true);
    QCOMPARE(m_endingTriggersSpy->count(), 1);
    QCOMPARE(m_endingTriggersSpy->at(0).at(0).value<TriggerTypes>(), TriggerType::Tap);

    QCOMPARE(m_handler->handleEvent(PointerButtonEvent(m_touchpad.get(), Qt::MouseButton::LeftButton, BTN_LEFT, true)), true);
    QCOMPARE(m_activatingTriggersSpy->count(), 2);
    QCOMPARE(m_activatingTriggersSpy->at(1).at(0).value<TriggerTypes>(), TriggerType::Tap);

    QCOMPARE(m_handler->handleEvent(PointerButtonEvent(m_touchpad.get(), Qt::MouseButton::LeftButton, BTN_LEFT, false)), true);
    QCOMPARE(m_endingTriggersSpy->count(), 2);
    QCOMPARE(m_endingTriggersSpy->at(1).at(0).value<TriggerTypes>(), TriggerType::Tap);
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
}

void TestTouchpadTriggerHandler::tap4_moved()
{
    m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Tap));

    addPoints(4);
    movePoints({0.1, 0.1});
    removePoints();

    QCOMPARE(m_activatingTriggersSpy->count(), 0);
}

void TestTouchpadTriggerHandler::tap4_slow()
{
    m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Tap));

    addPoints(4);
    QTest::qWait(500);
    removePoints();

    QCOMPARE(m_activatingTriggersSpy->count(), 0);
}

void TestTouchpadTriggerHandler::tap4_clicked()
{
    m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Click));
    m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Tap));

    addPoints(4);
    m_handler->handleEvent(TouchpadClickEvent(m_touchpad.get(), true));
    m_handler->handleEvent(TouchpadClickEvent(m_touchpad.get(), false));
    removePoints();

    QCOMPARE(m_activatingTriggersSpy->count(), 1);
    QCOMPARE(m_activatingTriggersSpy->at(0).at(0).value<TriggerTypes>(), TriggerType::Click);
}

void TestTouchpadTriggerHandler::tap_fingerCount_data()
{
    QTest::addColumn<int>("triggerFingers");
    QTest::addColumn<int>("fingers");
    QTest::addColumn<Qt::MouseButton>("libinputButton");
    QTest::addColumn<int>("libinputNativeButton");
    QTest::addColumn<bool>("lmrTapButtonMap");
    QTest::addColumn<bool>("activated");

    QTest::addRow("1") << 1 << 1 << Qt::MouseButton::LeftButton << BTN_LEFT << false << true;
    QTest::addRow("2") << 2 << 2 << Qt::MouseButton::RightButton << BTN_RIGHT << false << true;
    QTest::addRow("3") << 3 << 3 << Qt::MouseButton::MiddleButton << BTN_MIDDLE << false << true;
    QTest::addRow("4") << 4 << 4 << Qt::MouseButton::NoButton << 0 << false << true;
    QTest::addRow("5") << 5 << 5 << Qt::MouseButton::NoButton << 0 << false << true;
    QTest::addRow("wrong") << 2 << 1 << Qt::MouseButton::LeftButton << BTN_LEFT << false << false;
    QTest::addRow("1 lmr") << 1 << 1 << Qt::MouseButton::LeftButton << BTN_LEFT << true << true;
    QTest::addRow("2 lmr") << 2 << 2 << Qt::MouseButton::MiddleButton << BTN_MIDDLE << true << true;
    QTest::addRow("3 lmr") << 3 << 3 << Qt::MouseButton::RightButton << BTN_RIGHT << true << true;
    QTest::addRow("4 lmr") << 4 << 4 << Qt::MouseButton::NoButton << 0 << true << true;
    QTest::addRow("5 lmr") << 5 << 5 << Qt::MouseButton::NoButton << 0 << true << true;
}

void TestTouchpadTriggerHandler::tap_fingerCount()
{
    QFETCH(int, triggerFingers);
    QFETCH(int, fingers);
    QFETCH(Qt::MouseButton, libinputButton);
    QFETCH(int, libinputNativeButton);
    QFETCH(bool, lmrTapButtonMap);
    QFETCH(bool, activated);

    auto trigger = std::make_unique<Trigger>(TriggerType::Tap);
    trigger->m_activationCondition = std::make_shared<VariableCondition>("fingers", static_cast<qreal>(triggerFingers), ComparisonOperator::EqualTo);
    m_handler->addTrigger(std::move(trigger));
    m_touchpad->properties().setLmrTapButtonMap(lmrTapButtonMap);

    addPoints(fingers);
    removePoints();
    if (libinputButton) {
        m_handler->handleEvent(PointerButtonEvent(m_touchpad.get(), libinputButton, libinputNativeButton, true));
        m_handler->handleEvent(PointerButtonEvent(m_touchpad.get(), libinputButton, libinputNativeButton, false));
    }

    QCOMPARE(m_activatingTriggerSpy->count(), activated);
}

TouchPoint &TestTouchpadTriggerHandler::addPoint(const QPointF &position)
{
    auto &point = m_touchpad->m_touchPoints[m_touchpad->validTouchPoints().size()];
    point.valid = true;
    point.initialPosition = point.position = position;
    point.downTimestamp = std::chrono::steady_clock::now();
    m_handler->handleEvent(TouchEvent(m_touchpad.get(), InputEventType::TouchDown, point));
    return point;
}

void TestTouchpadTriggerHandler::addPoints(uint8_t count, const QPointF &position)
{
    for (auto i = 0; i < count; i++) {
        addPoint(position);
    }
}

void TestTouchpadTriggerHandler::movePoints(const QPointF &delta)
{
    for (auto &point : m_touchpad->m_touchPoints) {
        if (!point.valid) {
            continue;
        }

        point.position += delta;
        m_handler->handleEvent(TouchChangedEvent(m_touchpad.get(), point, delta));
    }
}

void TestTouchpadTriggerHandler::removePoints(int16_t count)
{
    uint8_t removed{};
    for (auto &point : std::ranges::reverse_view(m_touchpad->m_touchPoints)) {
        if (!point.valid) {
            continue;
        }

        point.valid = false;
        m_handler->handleEvent(TouchEvent(m_touchpad.get(), InputEventType::TouchUp, point));
        if (++removed == count) {
            break;
        }
    }
}

};

QTEST_MAIN(libinputactions::TestTouchpadTriggerHandler)
#include "TestTouchpadTriggerHandler.moc"