#pragma once

#include <QTest>

namespace InputActions
{

class Test : public QObject
{
    Q_OBJECT

public:
    static void initMain();
};

}