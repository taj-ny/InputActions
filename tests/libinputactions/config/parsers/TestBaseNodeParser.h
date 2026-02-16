#pragma once

#include "Test.h"

namespace InputActions
{

class TestBaseNodeParser : public Test
{
    Q_OBJECT

private slots:
    void boolean_valid__parsesNodeCorrectly_data();
    void boolean_valid__parsesNodeCorrectly();
    void boolean_invalid__throwsInvalidValueConfigException_data();
    void boolean_invalid__throwsInvalidValueConfigException();

    void int8_valid__parsesNodeCorrectly_data();
    void int8_valid__parsesNodeCorrectly();
    void int8_invalid__throwsInvalidValueConfigException_data();
    void int8_invalid__throwsInvalidValueConfigException();

    void uint8_valid__parsesNodeCorrectly_data();
    void uint8_valid__parsesNodeCorrectly();
    void uint8_invalid__throwsInvalidValueConfigException_data();
    void uint8_invalid__throwsInvalidValueConfigException();

    void uint32_valid__parsesNodeCorrectly_data();
    void uint32_valid__parsesNodeCorrectly();
    void uint32_invalid__throwsInvalidValueConfigException_data();
    void uint32_invalid__throwsInvalidValueConfigException();

    void uint64_valid__parsesNodeCorrectly_data();
    void uint64_valid__parsesNodeCorrectly();
    void uint64_invalid__throwsInvalidValueConfigException_data();
    void uint64_invalid__throwsInvalidValueConfigException();

    void qreal_valid__parsesNodeCorrectly_data();
    void qreal_valid__parsesNodeCorrectly();
    void qreal_invalid__throwsInvalidValueConfigException_data();
    void qreal_invalid__throwsInvalidValueConfigException();

    void string_valid__parsesNodeCorrectly_data();
    void string_valid__parsesNodeCorrectly();

    void map__throwsInvalidNodeTypeConfigException();
    void null__throwsInvalidNodeTypeConfigException();
    void sequence__throwsInvalidNodeTypeConfigException();
};

}