#pragma once

#include <QObject>

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

enum class CursorShape : uint32_t
{
    Alias,
    AllScroll,
    ColResize,
    Copy,
    Crosshair,
    Default,
    EResize,
    EWResize,
    Grab,
    Grabbing,
    Help,
    Move,
    NEResize,
    NESWResize,
    NResize,
    NSResize,
    NWResize,
    NWSEResize,
    NotAllowed,
    Pointer,
    Progress,
    RowResize,
    SEResize,
    SResize,
    SWResize,
    Text,
    UpArrow,
    WResize,
    Wait
};

enum class TriggerSpeed
{
    Any,
    Slow,
    Fast
};
Q_ENUM_NS(TriggerSpeed)

enum class TriggerType : uint32_t {
    None = 0,
    Pinch = 1u << 0,
    Press = 1u << 1,
    Rotate = 1u << 2,
    Stroke = 1u << 3,
    Swipe = 1u << 4,
    Wheel = 1u << 5,

    PinchRotate = Pinch | Rotate,
    StrokeSwipe = Stroke | Swipe,

    All = UINT32_MAX
};
Q_ENUM_NS(TriggerType)
Q_DECLARE_FLAGS(TriggerTypes, TriggerType)
Q_DECLARE_OPERATORS_FOR_FLAGS(TriggerTypes)

static inline TriggerTypes operator~(const TriggerTypes &value)
{
    return TriggerTypes(~static_cast<uint32_t>(value));
}
static inline TriggerType operator~(const TriggerType &value)
{
    return TriggerType(~static_cast<uint32_t>(value));
}

template<class... Ts>
struct overloads : Ts... { using Ts::operator()...; };

}