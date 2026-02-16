#include "TestBaseNodeParser.h"
#include <libinputactions/config/ConfigIssue.h>
#include <libinputactions/config/ConfigIssueManager.h>
#include <libinputactions/config/Node.h>

namespace InputActions
{

void TestBaseNodeParser::boolean_valid__parsesNodeCorrectly_data()
{
    QTest::addColumn<QString>("raw");
    QTest::addColumn<bool>("result");

    // Those are the only values officially defined as valid
    QTest::addRow("true") << "true" << true;
    QTest::addRow("false") << "false" << false;
}

void TestBaseNodeParser::boolean_valid__parsesNodeCorrectly()
{
    QFETCH(QString, raw);
    QFETCH(bool, result);

    const auto node = Node::create(raw);
    QCOMPARE(node->as<bool>(), result);
}

void TestBaseNodeParser::boolean_invalid__throwsInvalidValueConfigException_data()
{
    QTest::addColumn<QString>("raw");

    QTest::addRow("empty") << "";
    QTest::addRow("space") << "\" \"";
    QTest::addRow("char") << "a";
    QTest::addRow("number") << "1";
}

void TestBaseNodeParser::boolean_invalid__throwsInvalidValueConfigException()
{
    QFETCH(QString, raw);

    const auto node = Node::create(raw);
    INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<bool>(), InvalidValueConfigException, 0, 0);
}

void TestBaseNodeParser::int8_valid__parsesNodeCorrectly_data()
{
    QTest::addColumn<QString>("raw");
    QTest::addColumn<int8_t>("result");

    QTest::addRow("min") << "-128" << static_cast<int8_t>(-128);
    QTest::addRow("-1") << "-1" << static_cast<int8_t>(-1);
    QTest::addRow("0") << "0" << static_cast<int8_t>(0);
    QTest::addRow("1") << "1" << static_cast<int8_t>(1);
    QTest::addRow("max") << "127" << static_cast<int8_t>(127);
    QTest::addRow("quoted") << "\"1\"" << static_cast<int8_t>(1);
}

void TestBaseNodeParser::int8_valid__parsesNodeCorrectly()
{
    QFETCH(QString, raw);
    QFETCH(int8_t, result);

    const auto node = Node::create(raw);
    QCOMPARE(node->as<int8_t>(), result);
}

void TestBaseNodeParser::int8_invalid__throwsInvalidValueConfigException_data()
{
    QTest::addColumn<QString>("raw");

    QTest::addRow("empty") << "";
    QTest::addRow("space") << "\" \"";
    QTest::addRow("char") << "a";
    QTest::addRow("float") << "1.0";
    QTest::addRow("min-1") << "-129";
    QTest::addRow("max+1") << "128";
}

void TestBaseNodeParser::int8_invalid__throwsInvalidValueConfigException()
{
    QFETCH(QString, raw);

    const auto node = Node::create(raw);
    INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<int8_t>(), InvalidValueConfigException, 0, 0);
}

void TestBaseNodeParser::uint8_valid__parsesNodeCorrectly_data()
{
    QTest::addColumn<QString>("raw");
    QTest::addColumn<uint8_t>("result");

    QTest::addRow("min") << "0" << static_cast<uint8_t>(0);
    QTest::addRow("1") << "1" << static_cast<uint8_t>(1);
    QTest::addRow("max") << "255" << static_cast<uint8_t>(255);
    QTest::addRow("quoted") << "\"1\"" << static_cast<uint8_t>(1);
}

void TestBaseNodeParser::uint8_valid__parsesNodeCorrectly()
{
    QFETCH(QString, raw);
    QFETCH(uint8_t, result);

    const auto node = Node::create(raw);
    QCOMPARE(node->as<uint8_t>(), result);
}

void TestBaseNodeParser::uint8_invalid__throwsInvalidValueConfigException_data()
{
    QTest::addColumn<QString>("raw");

    QTest::addRow("empty") << "";
    QTest::addRow("space") << "\" \"";
    QTest::addRow("char") << "a";
    QTest::addRow("float") << "1.0";
    QTest::addRow("min-1") << "-1";
    QTest::addRow("max+1") << "256";
}

void TestBaseNodeParser::uint8_invalid__throwsInvalidValueConfigException()
{
    QFETCH(QString, raw);

    const auto node = Node::create(raw);
    INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<uint8_t>(), InvalidValueConfigException, 0, 0);
}

void TestBaseNodeParser::uint32_valid__parsesNodeCorrectly_data()
{
    QTest::addColumn<QString>("raw");
    QTest::addColumn<uint32_t>("result");

    QTest::addRow("min") << "0" << static_cast<uint32_t>(0);
    QTest::addRow("1") << "1" << static_cast<uint32_t>(1);
    QTest::addRow("max") << "4294967295" << static_cast<uint32_t>(4294967295);
    QTest::addRow("quoted") << "\"1\"" << static_cast<uint32_t>(1);
}

void TestBaseNodeParser::uint32_valid__parsesNodeCorrectly()
{
    QFETCH(QString, raw);
    QFETCH(uint32_t, result);

    const auto node = Node::create(raw);
    QCOMPARE(node->as<uint32_t>(), result);
}

void TestBaseNodeParser::uint32_invalid__throwsInvalidValueConfigException_data()
{
    QTest::addColumn<QString>("raw");

    QTest::addRow("empty") << "";
    QTest::addRow("space") << "\" \"";
    QTest::addRow("char") << "a";
    QTest::addRow("float") << "1.0";
    QTest::addRow("min-1") << "-1";
    QTest::addRow("max+1") << "4294967296";
}

void TestBaseNodeParser::uint32_invalid__throwsInvalidValueConfigException()
{
    QFETCH(QString, raw);

    const auto node = Node::create(raw);
    INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<uint32_t>(), InvalidValueConfigException, 0, 0);
}

void TestBaseNodeParser::uint64_valid__parsesNodeCorrectly_data()
{
    QTest::addColumn<QString>("raw");
    QTest::addColumn<uint64_t>("result");

    QTest::addRow("min") << "0" << static_cast<uint64_t>(0);
    QTest::addRow("1") << "1" << static_cast<uint64_t>(1);
    QTest::addRow("max") << "18446744073709551615" << static_cast<uint64_t>(18446744073709551615U);
    QTest::addRow("quoted") << "\"1\"" << static_cast<uint64_t>(1);
}

void TestBaseNodeParser::uint64_valid__parsesNodeCorrectly()
{
    QFETCH(QString, raw);
    QFETCH(uint64_t, result);

    const auto node = Node::create(raw);
    QCOMPARE(node->as<uint64_t>(), result);
}

void TestBaseNodeParser::uint64_invalid__throwsInvalidValueConfigException_data()
{
    QTest::addColumn<QString>("raw");

    QTest::addRow("empty") << "";
    QTest::addRow("space") << "\" \"";
    QTest::addRow("char") << "a";
    QTest::addRow("float") << "1.0";
    QTest::addRow("min-1") << "-1";
    QTest::addRow("max+1") << "18446744073709551616";
}

void TestBaseNodeParser::uint64_invalid__throwsInvalidValueConfigException()
{
    QFETCH(QString, raw);

    const auto node = Node::create(raw);
    INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<uint64_t>(), InvalidValueConfigException, 0, 0);
}

void TestBaseNodeParser::qreal_valid__parsesNodeCorrectly_data()
{
    QTest::addColumn<QString>("raw");
    QTest::addColumn<qreal>("result");

    QTest::addRow("-1") << "-1" << static_cast<qreal>(-1);
    QTest::addRow("0") << "0" << static_cast<qreal>(0);
    QTest::addRow("1") << "1" << static_cast<qreal>(1);

    QTest::addRow("trailingDot") << "2." << static_cast<qreal>(2);
    QTest::addRow("leadingDot") << ".2" << static_cast<qreal>(0.2);

    QTest::addRow("-123.456") << "-123.456" << static_cast<qreal>(-123.456);
    QTest::addRow("123.456") << "123.456" << static_cast<qreal>(123.456);

    QTest::addRow("quoted") << "\"1.1\"" << static_cast<qreal>(1.1);
}

void TestBaseNodeParser::qreal_valid__parsesNodeCorrectly()
{
    QFETCH(QString, raw);
    QFETCH(qreal, result);

    const auto node = Node::create(raw);
    QCOMPARE(node->as<qreal>(), result);
}

void TestBaseNodeParser::qreal_invalid__throwsInvalidValueConfigException_data()
{
    QTest::addColumn<QString>("raw");

    QTest::addRow("empty") << "";
    QTest::addRow("space") << "\" \"";
    QTest::addRow("char") << "a";
}

void TestBaseNodeParser::qreal_invalid__throwsInvalidValueConfigException()
{
    QFETCH(QString, raw);

    const auto node = Node::create(raw);
    INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<qreal>(), InvalidValueConfigException, 0, 0);
}

void TestBaseNodeParser::string_valid__parsesNodeCorrectly_data()
{
    QTest::addColumn<QString>("raw");

    QTest::addRow("bool") << "true";
    QTest::addRow("number") << "2";
    QTest::addRow("float") << "2";
}

void TestBaseNodeParser::string_valid__parsesNodeCorrectly()
{
    QFETCH(QString, raw);

    const auto node = Node::create(raw);
    QCOMPARE(node->as<QString>(), raw);
}

void TestBaseNodeParser::map__throwsInvalidNodeTypeConfigException()
{
    const auto node = Node::create("_: _");

    INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION_SAVE(node->as<bool>(), InvalidNodeTypeConfigException, 0, 0, e);
    QCOMPARE(e->expected(), NodeType::Scalar);
    QCOMPARE(e->actual(), NodeType::Map);
}

void TestBaseNodeParser::null__throwsInvalidNodeTypeConfigException()
{
    const auto node = Node::create("_:")->at("_")->shared_from_this();
    INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(node->as<bool>(), InvalidValueConfigException, 0, 0);
}

void TestBaseNodeParser::sequence__throwsInvalidNodeTypeConfigException()
{
    const auto node = Node::create("[]");

    INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION_SAVE(node->as<bool>(), InvalidNodeTypeConfigException, 0, 0, e);
    QCOMPARE(e->expected(), NodeType::Scalar);
    QCOMPARE(e->actual(), NodeType::Sequence);
}

}

QTEST_MAIN(InputActions::TestBaseNodeParser)