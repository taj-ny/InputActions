#pragma once

#include <QTest>

namespace libinputactions
{

class TestValue : public QObject
{
    Q_OBJECT

private slots:
    void init();

    void get_defaultConstructor_returnsNullopt();
    void get_valueConstructor();

    void get_command();
    void get_commandNullValue_returnsNullopt();

    void get_function();

    void get_existentVariable();
    void get_nonExistentVariable_returnsNullopt();
};

}