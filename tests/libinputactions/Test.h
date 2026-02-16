#pragma once

#include <QTest>

namespace InputActions
{

#define INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION(action, exceptionType, line, column) \
    try {                                                                                \
        action;                                                                          \
        QFAIL("Expected " #exceptionType " to be thrown.");                              \
    } catch (const exceptionType &e) {                                                   \
        QCOMPARE(e.position(), TextPosition(line, column));                              \
    }

#define INPUTACTIONS_VERIFY_THROWS_CONFIG_EXCEPTION_SAVE(action, exceptionType, line, column, var) \
    std::optional<exceptionType> var;                                                              \
    try {                                                                                          \
        action;                                                                                    \
        QFAIL("Expected " #exceptionType " to be thrown.");                                        \
    } catch (const exceptionType &e_) {                                                            \
        QCOMPARE(e_.position(), TextPosition(line, column));                                       \
        var = e_;                                                                                  \
    }

#define INPUTACTIONS_VERIFY_ADDS_CONFIG_ISSUE_SAVE(action, issueType, line, column, var) \
    g_configIssueManager = std::make_shared<ConfigIssueManager>();                       \
    action;                                                                              \
    const auto *var = g_configIssueManager->findIssueByType<issueType>();                \
    QVERIFY(var);                                                                        \
    QCOMPARE(var->position(), TextPosition(line, column));

#define INPUTACTIONS_VERIFY_ADDS_CONFIG_ISSUE(action, issueType, line, column)          \
    INPUTACTIONS_VERIFY_ADDS_CONFIG_ISSUE_SAVE(action, issueType, line, column, issue_)

class Test : public QObject
{
    Q_OBJECT

public:
    static void initMain();
};

}