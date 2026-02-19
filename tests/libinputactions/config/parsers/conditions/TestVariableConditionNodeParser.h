#pragma once

#include "Test.h"

namespace InputActions
{

class TestVariableConditionNodeParser : public Test
{
    Q_OBJECT

private slots:
    void init();

    void boolVariableWithoutOperator__parsesNodeCorrectly();
    void negatedBoolVariableWithoutOperator__parsesNodeCorrectly();

    void negated__parsesNodeCorrectly();

    void between__parsesNodeCorrectly();
    void between_point__parsesNodeCorrectly();
    void between_invalid_oneValue__throwsInvalidValueConfigException();
    void between_invalid_threeValues__throwsInvalidValueConfigException();

    void contains_string__parsesNodeCorrectly();
    void contains_flags_sequence__parsesNodeCorrectly();
    void contains_flags_scalar__parsesNodeCorrectly();

    void oneOf_sequence__parsesNodeCorrectly();
    void oneOf_scalar__parsesNodeCorrectly();

    void matches__parsesNodeCorrectly();
    void matches_invalidRegex__throwsInvalidValueConfigException();

    void simpleOperators__parsesNodeCorrectly_data();
    void simpleOperators__parsesNodeCorrectly();

    void inGroups__variableManagerPropagated_doesNotThrow();

    void invalid_invalidVariable__throwsInvalidValueConfigException();
    void invalid_noOperator__throwsInvalidValueConfigException();
    void invalid_noValue__throwsInvalidValueConfigException();
};

}