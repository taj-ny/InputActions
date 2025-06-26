#pragma once

#include <QLoggingCategory>
#include <QObject>

#define INPUTACTIONS_DECLARE_SINGLETON(T)                     \
    public:                                                   \
        static T *instance();                                 \
        static void setInstance(std::shared_ptr<T> instance); \
                                                              \
    private:                                                  \
        static std::shared_ptr<T> s_instance;

#define INPUTACTIONS_SINGLETON_IMPL(T, DEFAULT)                   \
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
    std::shared_ptr<T> T::s_instance = DEFAULT;

#define INPUTACTIONS_SINGLETON(T) INPUTACTIONS_SINGLETON_IMPL(T, std::shared_ptr<T>(new T))
#define INPUTACTIONS_SINGLETON_NODEFAULT(T) INPUTACTIONS_SINGLETON_IMPL(T, nullptr)

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

static inline TriggerType operator~(const TriggerType &value)
{
    return TriggerType(~static_cast<uint32_t>(value));
}

template<class... Ts>
struct overloads : Ts... { using Ts::operator()...; };

}