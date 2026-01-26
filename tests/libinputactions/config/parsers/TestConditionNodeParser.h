#pragma once

#include "Test.h"

namespace InputActions
{

class TestConditionNodeParser : public Test
{
    Q_OBJECT

private slots:
    void init();

    void group_all__parsedCorrectly();
    void group_any__parsedCorrectly();
    void group_none__parsedCorrectly();

    void group_implicitAll__parsedAsAllGroup();

    void variable_boolVariableWithoutOperator__parsedCorrectly();
    void variable_negatedBoolVariableWithoutOperator__parsedCorrectly();
    void variable_matchesOperator_invalidRegex__throwsInvalidValueConfigException();
    void variable_invalidVariable__throwsInvalidVariableConfigException();
    void variable_inGroups__variableManagerPropagated_doesNotThrow();

    void legacy_windowClass__parsedCorrectly();
    void legacy_windowClass__addsDeprecatedFeatureIssue();
    void legacy_windowClass_negated__parsedCorrectly();
    void legacy_windowState__parsedCorrectly();
    void legacy_windowState__addsDeprecatedFeatureIssue();
    void legacy_windowState_negated__parsedCorrectly();

    void legacy_and__parsedAsAllGroup();
    void legacy_or__parsedAsAnyGroup();

    void legacy_mixedWithNormal_normalFirst__throwsInvalidValueConfigException();
    void legacy_mixedWithNormal_legacyFirst__throwsInvalidValueConfigException();
};

}