#pragma once

#include <QLoggingCategory>
#include <QObject>

#define INPUTACTIONS_DECLARE_SINGLETON(T)                  \
    public:                                                   \
        static T *instance();                                 \
        static void setInstance(std::shared_ptr<T> instance); \
                                                              \
    private:                                                  \
        static std::shared_ptr<T> s_instance;

#define INPUTACTIONS_SINGLETON(T)                              \
    T *T::instance()                                              \
    {                                                             \
        return s_instance.get();                                  \
    }                                                             \
                                                                  \
    void T::setInstance(std::shared_ptr<T> instance)              \
    {                                                             \
        s_instance = std::move(instance);                         \
    }                                                             \
                                                                  \
    std::shared_ptr<T> T::s_instance = std::shared_ptr<T>(new T);

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