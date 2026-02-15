#include "TestRange.h"

namespace InputActions
{

void TestRange::contains_qreal_data()
{
    QTest::addColumn<std::optional<qreal>>("min");
    QTest::addColumn<std::optional<qreal>>("max");
    QTest::addColumn<int>("value");
    QTest::addColumn<bool>("result");

    QTest::addRow("none, 0") << std::optional<qreal>() << std::optional<qreal>() << 0 << true;
    QTest::addRow("none, positive") << std::optional<qreal>() << std::optional<qreal>() << 1 << true;
    QTest::addRow("none, negative") << std::optional<qreal>() << std::optional<qreal>() << -1 << true;
    QTest::addRow("min, below") << std::optional<qreal>{1} << std::optional<qreal>() << -1 << false;
    QTest::addRow("min, equal") << std::optional<qreal>{1} << std::optional<qreal>() << 1 << true;
    QTest::addRow("min, above") << std::optional<qreal>{1} << std::optional<qreal>() << 2 << true;
    QTest::addRow("max, below") << std::optional<qreal>() << std::optional<qreal>{1} << -1 << true;
    QTest::addRow("max, equal") << std::optional<qreal>() << std::optional<qreal>{1} << 1 << true;
    QTest::addRow("max, above") << std::optional<qreal>() << std::optional<qreal>{1} << 2 << false;
    QTest::addRow("minmax, below") << std::optional<qreal>{-1} << std::optional<qreal>{1} << -2 << false;
    QTest::addRow("minmax, equal to min") << std::optional<qreal>{-1} << std::optional<qreal>{1} << -1 << true;
    QTest::addRow("minmax, between") << std::optional<qreal>{-1} << std::optional<qreal>{1} << 0 << true;
    QTest::addRow("minmax, equal to max") << std::optional<qreal>{-1} << std::optional<qreal>{1} << 1 << true;
    QTest::addRow("minmax, above") << std::optional<qreal>{-1} << std::optional<qreal>{1} << 2 << false;
}

void TestRange::contains_qreal()
{
    QFETCH(std::optional<qreal>, min);
    QFETCH(std::optional<qreal>, max);
    QFETCH(int, value);
    QFETCH(bool, result);

    QCOMPARE(Range<qreal>(min, max).contains(value), result);
}

}

QTEST_MAIN(InputActions::TestRange)