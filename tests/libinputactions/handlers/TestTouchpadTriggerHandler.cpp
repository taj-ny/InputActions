#include "TestTouchpadTriggerHandler.h"
#include <QSignalSpy>
#include <QTest>
#include <ranges>
#include <libinputactions/handlers/TouchpadTriggerHandler.h>
#include <linux/input-event-codes.h>

namespace libinputactions
{

void TestTouchpadTriggerHandler::init()
{
    m_handler = std::make_unique<TouchpadTriggerHandler>();
    m_activatingTriggersSpy = std::make_unique<QSignalSpy>(m_handler.get(), &TriggerHandler::activatingTriggers);
    m_cancellingTriggersSpy = std::make_unique<QSignalSpy>(m_handler.get(), &TriggerHandler::cancellingTriggers);
    m_endingTriggersSpy = std::make_unique<QSignalSpy>(m_handler.get(), &TriggerHandler::endingTriggers);
    m_touchpad = std::make_unique<InputDevice>(InputDeviceType::Touchpad);
    m_touchpad->m_touchPoints = std::vector<TouchPoint>(5);
}

void TestTouchpadTriggerHandler::clickLeft_notTap()
{
    m_handler->handleEvent(TouchpadClickEvent(m_touchpad.get(), true));
    m_handler->handleEvent(PointerButtonEvent(m_touchpad.get(), Qt::MouseButton::LeftButton, BTN_LEFT, true));
    m_handler->handleEvent(TouchpadClickEvent(m_touchpad.get(), false));
    m_handler->handleEvent(PointerButtonEvent(m_touchpad.get(), Qt::MouseButton::LeftButton, BTN_LEFT, false));

    QCOMPARE(m_activatingTriggersSpy->count(), 1);
    QCOMPARE(m_activatingTriggersSpy->at(0).at(0).value<TriggerTypes>(), TriggerType::Click);
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
    QCOMPARE(m_handler->handleEvent(PointerButtonEvent(m_touchpad.get(), Qt::MouseButton::LeftButton, BTN_LEFT, true)), true);
    QCOMPARE(m_handler->handleEvent(PointerButtonEvent(m_touchpad.get(), Qt::MouseButton::LeftButton, BTN_LEFT, false)), true);

    QCOMPARE(m_activatingTriggersSpy->count(), 2);
    QCOMPARE(m_activatingTriggersSpy->at(1).at(0).value<TriggerTypes>(), TriggerType::Tap);
}

void TestTouchpadTriggerHandler::tap4()
{
    m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Tap));

    addPoints(4);
    removePoints();

    QCOMPARE(m_activatingTriggersSpy->count(), 1);
    QCOMPARE(m_activatingTriggersSpy->at(0).at(0).value<TriggerTypes>(), TriggerType::Tap);
}

void TestTouchpadTriggerHandler::tap4_moved()
{
    m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Tap));

    auto points = addPoints(4);
    for (auto point : points) {
        point->position += {0.1, 0.1};
        m_handler->handleEvent(TouchChangedEvent(m_touchpad.get(), *point, {0.1, 0.1}, 0));
    }
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

TouchPoint &TestTouchpadTriggerHandler::addPoint(const QPointF &position)
{
    auto &point = m_touchpad->m_touchPoints[m_touchpad->validTouchPoints()];
    point.valid = true;
    point.initialPosition = point.position = position;
    point.downTimestamp = std::chrono::steady_clock::now();
    m_handler->handleEvent(TouchEvent(m_touchpad.get(), InputEventType::TouchDown, point));
    return point;
}

std::vector<TouchPoint *> TestTouchpadTriggerHandler::addPoints(uint8_t count, const QPointF &position)
{
    std::vector<TouchPoint *> points;
    for (auto i = 0; i < count; i++) {
        points.push_back(&addPoint(position));
    }
    return points;
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