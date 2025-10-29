#pragma once

#include <QTest>

namespace libinputactions
{

class Test : public QObject
{
    Q_OBJECT

public:
    static void initMain();
};

}