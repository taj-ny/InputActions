#include "TestValue.h"
#include <libinputactions/variables/VariableManager.h>
#include <libinputactions/Value.h>

namespace libinputactions
{

void TestValue::init()
{
    g_variableManager = std::make_shared<VariableManager>();
}

void TestValue::get_defaultConstructor_returnsNullopt()
{
    const Value<bool> value;
    QVERIFY(!value.get().has_value());
}

void TestValue::get_valueConstructor()
{
    const Value<bool> value(true);
    QVERIFY(value.get().value());
}

void TestValue::get_command()
{
    const auto value = Value<QString>::command(Value<QString>("echo -n a"));
    QCOMPARE(value.get().value(), "a");
}

void TestValue::get_commandNullValue_returnsNullopt()
{
    const auto value = Value<QString>::command({});
    QVERIFY(!value.get().has_value());
}

void TestValue::get_function()
{
    const auto value = Value<bool>::function([]() { return true; });
    QVERIFY(value.get().value());
}

void TestValue::get_existentVariable()
{
    g_variableManager->registerRemoteVariable<bool>("_test", [](auto &value) { value = true; });
    const auto value = Value<bool>::variable("_test");
    QVERIFY(value.get().value());
}

void TestValue::get_nonExistentVariable_returnsNullopt()
{
    const auto value = Value<bool>::variable("_test");
    QVERIFY(!value.get().has_value());
}

}

QTEST_MAIN(libinputactions::TestValue)
#include "TestValue.moc"