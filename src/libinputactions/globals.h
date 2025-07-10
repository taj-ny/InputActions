#pragma once

#include <QLoggingCategory>
#include <QObject>

Q_DECLARE_LOGGING_CATEGORY(INPUTACTIONS)

namespace libinputactions
{
Q_NAMESPACE

enum class ComparisonOperator
{
    EqualTo,
    NotEqualTo,

    // List (right only)
    OneOf,

    // Number
    GreaterThan,
    GreaterThanOrEqual,
    LessThan,
    LessThanOrEqual,
    Between,

    // String
    Contains,
    Regex
};

enum class TriggerSpeed
{
    Any,
    Slow,
    Fast
};
Q_ENUM_NS(TriggerSpeed)

enum class TriggerType : uint32_t
{
    None = 0,
    Click = 1u << 0,
    Pinch = 1u << 1,
    Press = 1u << 2,
    Rotate = 1u << 3,
    Stroke = 1u << 4,
    Swipe = 1u << 5,
    Wheel = 1u << 6,

    PinchRotate = Pinch | Rotate,
    StrokeSwipe = Stroke | Swipe,

    All = UINT32_MAX
};
Q_ENUM_NS(TriggerType)
Q_DECLARE_FLAGS(TriggerTypes, TriggerType)
Q_DECLARE_OPERATORS_FOR_FLAGS(TriggerTypes)

static inline TriggerType operator~(TriggerType value)
{
    return TriggerType(~static_cast<uint32_t>(value));
}

template<class... Ts>
struct overloads : Ts...
{
    using Ts::operator()...;
};

}