#pragma once

#include "Test.h"

namespace InputActions
{

class TestTriggerNodeParser : public Test
{
    Q_OBJECT

private slots:
    void stroke_withConflictingBeginAction__throwsInvalidValueContextConfigException();
    void stroke_withNonConflictingBeginAction__doesNotThrow();
    void stroke_withEndAction__doesNotThrow();
    void stroke_invalidStroke__throwsInvalidValueConfigException();

    void swipe_angle__parsesNodeCorrectly();
    void swipe_direction__doesNotThrow();
    void swipe_invalidAngle__throwsInvalidValueConfigException();

    void fingers__parsesNodeCorrectly();
    void fingers_range__parsesNodeCorrectly();

    void mouseButtons_duplicateItem__throwsDuplicateSetItemConfigException();

    void keyboardModifiers__addsDeprecatedFeatureConfigIssue();
    void keyboardModifiers_any__doesNotAddCondition();
    void keyboardModifiers_metaAlt__parsesNodeCorrectly();
    void keyboardModifiers_none__parsesNodeCorrectly();
    void keyboardModifiers_invalid__throwsInvalidValueConfigException();

    void fingers_keyboardModifiers_triggerCondition__mergedIntoAllGroup();

    void invalid_noType__throwsMissingRequiredPropertyConfigException();
    void invalid_invalidType__throwsInvalidValueConfigException();
};

}