#pragma once

#include <libinputactions/actions/action.h>

#include <gmock/gmock.h>

namespace libinputactions
{

class MockGestureAction : public TriggerAction
{
public:
    MockGestureAction() = default;

    MOCK_METHOD(void, execute, (), (override));
    MOCK_METHOD(bool, canExecute, (), (const, override));
    MOCK_METHOD(const bool &, executed, (), (const, override));
    MOCK_METHOD(void, tryExecute, (), (override));

    MOCK_METHOD(void, triggerStarted, (), (override));
    MOCK_METHOD(void, triggerUpdated, (const qreal &, const QPointF &), (override));
    MOCK_METHOD(void, triggerEnded, (), (override));
    MOCK_METHOD(void, triggerCancelled, (), (override));
};

}