#pragma once

#include "Test.h"
#include <libinputactions/actions/Action.h>

namespace libinputactions
{

class TestAction : public Test
{
    Q_OBJECT

private slots:
    void init();

    void canExecute_noLimit();
    void canExecute_withLimit();

private:
    std::shared_ptr<Action> m_action;
};

}