#pragma once

#include <QTest>
#include <libinputactions/Range.h>

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