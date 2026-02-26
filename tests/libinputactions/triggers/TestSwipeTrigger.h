#pragma once

#include "Test.h"

namespace InputActions
{

class TestSwipeTrigger : public Test
{
    Q_OBJECT

private slots:
    void canUpdate_currentAngleDoesNotMatchRange_averageAngleMatchesRange__returnsTrue();
    void canUpdate_currentAngleMatchesRange_averageAngleDoesNotMatchRange__returnsFalse();

    void canUpdate_data();
    void canUpdate();

    void updateActions_bidirectional_normalAngle__updatesActionsWithPositiveDelta();
    void updateActions_bidirectional_oppositeAngle__updatesActionsWithNegativeDelta();
    void updateActions_bidirectional_overlappingAngleRanges__normalRangeHasHigherPriority();

    void matchesAngleRange_data();
    void matchesAngleRange();

    void matchesOppositeAngleRange_data();
    void matchesOppositeAngleRange();
};

}