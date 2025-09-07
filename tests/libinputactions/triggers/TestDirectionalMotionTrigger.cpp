#include "TestDirectionalMotionTrigger.h"
#include <libinputactions/triggers/DirectionalMotionTrigger.h>

namespace libinputactions
{

void TestDirectionalMotionTrigger::canUpdate_data()
{
    QTest::addColumn<uint32_t>("direction");
    QTest::addColumn<std::vector<uint32_t>>("valid");
    QTest::addColumn<std::vector<uint32_t>>("invalid");

    QTest::addRow("pinch any") << static_cast<TriggerDirection>(PinchDirection::Any)
                               << std::vector<uint32_t>({static_cast<TriggerDirection>(PinchDirection::In), static_cast<TriggerDirection>(PinchDirection::Out)})
                               << std::vector<uint32_t>();
    QTest::addRow("pinch in") << static_cast<TriggerDirection>(PinchDirection::In) << std::vector<uint32_t>({static_cast<TriggerDirection>(PinchDirection::In)})
                              << std::vector<uint32_t>({static_cast<TriggerDirection>(PinchDirection::Out)});
    QTest::addRow("pinch out") << static_cast<TriggerDirection>(PinchDirection::Out)
                               << std::vector<uint32_t>({static_cast<TriggerDirection>(PinchDirection::Out)})
                               << std::vector<uint32_t>({static_cast<TriggerDirection>(PinchDirection::In)});
    // Rotate is same as pinch
    QTest::addRow("swipe left") << static_cast<TriggerDirection>(SwipeDirection::Left)
                                << std::vector<uint32_t>({static_cast<TriggerDirection>(SwipeDirection::Left)})
                                << std::vector<uint32_t>({static_cast<TriggerDirection>(SwipeDirection::Right),
                                                          static_cast<TriggerDirection>(SwipeDirection::Up),
                                                          static_cast<TriggerDirection>(SwipeDirection::Down)});
    QTest::addRow("swipe right") << static_cast<TriggerDirection>(SwipeDirection::Right)
                                 << std::vector<uint32_t>({static_cast<TriggerDirection>(SwipeDirection::Right)})
                                 << std::vector<uint32_t>({static_cast<TriggerDirection>(SwipeDirection::Left),
                                                           static_cast<TriggerDirection>(SwipeDirection::Up),
                                                           static_cast<TriggerDirection>(SwipeDirection::Down)});
    QTest::addRow("swipe left/right")
        << static_cast<TriggerDirection>(SwipeDirection::LeftRight)
        << std::vector<uint32_t>({static_cast<TriggerDirection>(SwipeDirection::Left), static_cast<TriggerDirection>(SwipeDirection::Right)})
        << std::vector<uint32_t>({static_cast<TriggerDirection>(SwipeDirection::Up), static_cast<TriggerDirection>(SwipeDirection::Down)});
    QTest::addRow("swipe up") << static_cast<TriggerDirection>(SwipeDirection::Up) << std::vector<uint32_t>({static_cast<TriggerDirection>(SwipeDirection::Up)})
                              << std::vector<uint32_t>({static_cast<TriggerDirection>(SwipeDirection::Right),
                                                        static_cast<TriggerDirection>(SwipeDirection::Left),
                                                        static_cast<TriggerDirection>(SwipeDirection::Down)});
    QTest::addRow("swipe down") << static_cast<TriggerDirection>(SwipeDirection::Down)
                                << std::vector<uint32_t>({static_cast<TriggerDirection>(SwipeDirection::Down)})
                                << std::vector<uint32_t>({static_cast<TriggerDirection>(SwipeDirection::Right),
                                                          static_cast<TriggerDirection>(SwipeDirection::Up),
                                                          static_cast<TriggerDirection>(SwipeDirection::Left)});
    QTest::addRow("swipe up/down")
        << static_cast<TriggerDirection>(SwipeDirection::UpDown)
        << std::vector<uint32_t>({static_cast<TriggerDirection>(SwipeDirection::Up), static_cast<TriggerDirection>(SwipeDirection::Down)})
        << std::vector<uint32_t>({static_cast<TriggerDirection>(SwipeDirection::Left), static_cast<TriggerDirection>(SwipeDirection::Right)});
    QTest::addRow("swipe any") << static_cast<TriggerDirection>(SwipeDirection::Any)
                               << std::vector<uint32_t>({static_cast<TriggerDirection>(SwipeDirection::Up),
                                                         static_cast<TriggerDirection>(SwipeDirection::Down),
                                                         static_cast<TriggerDirection>(SwipeDirection::Left),
                                                         static_cast<TriggerDirection>(SwipeDirection::Right)})
                               << std::vector<uint32_t>({});
}

void TestDirectionalMotionTrigger::canUpdate()
{
    QFETCH(uint32_t, direction);
    QFETCH(std::vector<uint32_t>, valid);
    QFETCH(std::vector<uint32_t>, invalid);

    auto trigger = std::make_unique<DirectionalMotionTrigger>(TriggerType::None, direction);
    for (const auto &validDirection : valid) {
        DirectionalMotionTriggerUpdateEvent event;
        event.m_direction = validDirection;
        QVERIFY(trigger->canUpdate(event));
    }
    for (const auto &invalidDirection : invalid) {
        DirectionalMotionTriggerUpdateEvent event;
        event.m_direction = invalidDirection;
        QVERIFY(!trigger->canUpdate(event));
    }
}

}

QTEST_MAIN(libinputactions::TestDirectionalMotionTrigger)
#include "TestDirectionalMotionTrigger.moc"