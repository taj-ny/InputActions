#include <QSignalSpy>
#include <QTest>
#include <libinputactions/handlers/TouchpadTriggerHandler.h>

namespace libinputactions
{

class TestTouchpadTriggerHandler : public QObject
{
    Q_OBJECT

private slots:
    void init();

    void click_withoutLibinputButton();
    void click_withLibinputButton_data();
    void click_withLibinputButton();

    void press1_notDelayedOrBlocked();
    void press1_hasClickTrigger_delayed();
    void press1_hasTapTrigger_delayed();
    void press1_clickedDuringPress_pressCancelledAndClickActivated();
    void press2_notDelayedOrBlocked();
    void press3_blocked();

    void swipe2();

    void tap1();
    void tap1_tappedAgainBeforeLibinputButtonReleased();
    void tap4();
    void tap4_moved();
    void tap4_slow();
    void tap4_clicked();

    void tap_fingerCount_data();
    void tap_fingerCount();

private:
    TouchPoint &addPoint(const QPointF &position = {0.5, 0.5});
    void addPoints(uint8_t count, const QPointF &position = {0.5, 0.5});
    void movePoints(const QPointF &delta);
    void removePoints(int16_t count = -1);

    std::unique_ptr<TouchpadTriggerHandler> m_handler;
    std::unique_ptr<QSignalSpy> m_activatingTriggerSpy;
    std::unique_ptr<QSignalSpy> m_activatingTriggersSpy;
    std::unique_ptr<QSignalSpy> m_cancellingTriggersSpy;
    std::unique_ptr<QSignalSpy> m_endingTriggersSpy;

    std::unique_ptr<InputDevice> m_touchpad;
};

};