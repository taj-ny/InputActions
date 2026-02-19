#include "TestSeparatedStringNodeParser.h"
#include <libinputactions/config/Node.h>
#include <libinputactions/config/parsers/separated-string.h>
#include <libinputactions/utils/StringUtils.h>

namespace InputActions
{

void TestSeparatedStringNodeParser::number__parsesNodeCorrectly()
{
    const auto node = Node::create("12.34,43.21");
    const auto values = parseSeparatedString2<qreal>(node.get(), ',');

    QCOMPARE(values.first, 12.34);
    QCOMPARE(values.second, 43.21);
}

void TestSeparatedStringNodeParser::point__parsesNodeCorrectly()
{
    const auto node = Node::create("12.34,43.21;67.89,98.76");
    const auto values = parseSeparatedString2<QPointF>(node.get(), ';');

    QCOMPARE(values.first, QPointF(12.34, 43.21));
    QCOMPARE(values.second, QPointF(67.89, 98.76));
}

void TestSeparatedStringNodeParser::string__parsesNodeCorrectly()
{
    const auto node = Node::create("ab,ba");
    const auto values = parseSeparatedString2<QString>(node.get(), ',');

    QCOMPARE(values.first, "ab");
    QCOMPARE(values.second, "ba");
}

void TestSeparatedStringNodeParser::invalid__throwsInvalidValueConfigException_data()
{
    QTest::addColumn<QString>("raw");

    QTest::addRow("empty") << "";
    QTest::addRow("space") << " ";
    QTest::addRow("leftOnly") << "a,";
    QTest::addRow("rightOnly") << ",a";
}

void TestSeparatedStringNodeParser::invalid__throwsInvalidValueConfigException()
{
    QFETCH(QString, raw);

    const auto node = Node::create(StringUtils::quoted(raw));
    INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(parseSeparatedString2<QString>(node.get(), ','), InvalidValueConfigException, 0, 0);
}

}

QTEST_MAIN(InputActions::TestSeparatedStringNodeParser)