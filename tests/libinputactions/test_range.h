#pragma once

#include <libinputactions/Range.h>

#include <QTest>

namespace libinputactions
{

class TestRange : public QObject
{
    Q_OBJECT

private slots:
    void contains_qreal_data();
    void contains_qreal();
};

}