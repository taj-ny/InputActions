#pragma once

#include <QTest>
#include <libinputactions/triggers/Trigger.h>

namespace libinputactions
{

class TestTrigger : public QObject
{
    Q_OBJECT

private slots:
    void init();

    void canActivate_mouseButtons_data();
    void canActivate_mouseButtons();

    void update_threshold_data();
    void update_threshold();

private:
    std::unique_ptr<Trigger> m_trigger;
};

}