#pragma once

#include <QTest>

namespace libinputactions
{

class TestActionInterval : public QObject
{
    Q_OBJECT

private slots:
    void matches_data();
    void matches();
};

}