#pragma once

#include "Test.h"
#include <libinputactions/actions/ActionExecutor.h>

namespace libinputactions
{

class TestActionExecutor : public Test
{
    Q_OBJECT

private slots:
    void init();

    void execute_syncAction_executedOnMainThread();
    void execute_asyncAction_executedOnActionThread();
    void execute_syncActionWhileActionThreadIsBusy_executedOnActionThread();
    void execute_syncAndAsyncActions_orderPreserved();

private:
    static bool isMainThread();

    std::unique_ptr<ActionExecutor> m_executor;
};

}