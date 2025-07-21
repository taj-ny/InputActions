#pragma once

#include <QTest>

namespace libinputactions
{

class TestTriggerAction : public QObject
{
    Q_OBJECT

private slots:
    void triggerUpdated_intervals_data();
    void triggerUpdated_intervals();
};

}