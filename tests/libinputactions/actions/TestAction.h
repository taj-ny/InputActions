#pragma once

#include <QTest>
#include <libinputactions/actions/Action.h>

namespace libinputactions
{

class TestAction : public QObject
{
    Q_OBJECT

private slots:
    void init();

    void canExecute_noLimit();
    void canExecute_withLimit();

private:
    Action m_action;
};

}