/*
    Input Actions - Input handler that executes user-defined actions
    Copyright (C) 2024-2026 Marcin Woźniak

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include "yaml-cpp/yaml.h"
#include <QRegularExpression>
#include <QVector>
#include <libinputactions/Value.h>
#include <libinputactions/actions/ActionGroup.h>
#include <libinputactions/actions/CommandAction.h>
#include <libinputactions/actions/InputAction.h>
#include <libinputactions/actions/PlasmaGlobalShortcutAction.h>
#include <libinputactions/actions/SleepAction.h>
#include <libinputactions/actions/TriggerAction.h>
#include <libinputactions/conditions/ConditionGroup.h>
#include <libinputactions/conditions/VariableCondition.h>
#include <libinputactions/handlers/KeyboardTriggerHandler.h>
#include <libinputactions/handlers/MouseTriggerHandler.h>
#include <libinputactions/handlers/PointerTriggerHandler.h>
#include <libinputactions/handlers/TouchpadTriggerHandler.h>
#include <libinputactions/handlers/TouchscreenTriggerHandler.h>
#include <libinputactions/input/KeyboardKey.h>
#include <libinputactions/input/MouseButton.h>
#include <libinputactions/input/backends/InputBackend.h>
#include <libinputactions/input/devices/InputDeviceRule.h>
#include <libinputactions/interfaces/CursorShapeProvider.h>
#include <libinputactions/triggers/HoverTrigger.h>
#include <libinputactions/triggers/KeyboardShortcutTrigger.h>
#include <libinputactions/triggers/PressTrigger.h>
#include <libinputactions/triggers/StrokeTrigger.h>
#include <libinputactions/triggers/WheelTrigger.h>
#include <libinputactions/variables/Variable.h>
#include <libinputactions/variables/VariableManager.h>
#include <unordered_set>

// Most of the code below is garbage

using namespace InputActions;

namespace YAML
{

static const Node asSequence(const Node &node)
{
    Node result(NodeType::Sequence);
    if (node.IsSequence()) {
        for (const auto &child : node) {
            result.push_back(child);
        }
    } else {
        result.push_back(node);
    }
    return result;
}

static InputActions::Value<std::any> asAny(const Node &node, const std::type_index &type)
{
    if (type == typeid(bool)) {
        return node.as<InputActions::Value<bool>>();
    } else if (type == typeid(CursorShape)) {
        return node.as<InputActions::Value<CursorShape>>();
    } else if (type == typeid(Qt::KeyboardModifiers)) {
        return InputActions::Value<Qt::KeyboardModifiers>(asSequence(node).as<Qt::KeyboardModifiers>()); // TODO
    } else if (type == typeid(InputDeviceTypes)) {
        return InputActions::Value<InputDeviceTypes>(asSequence(node).as<InputDeviceTypes>()); // TODO
    } else if (type == typeid(qreal)) {
        return node.as<InputActions::Value<qreal>>();
    } else if (type == typeid(QPointF)) {
        return node.as<InputActions::Value<QPointF>>();
    } else if (type == typeid(QString)) {
        return node.as<InputActions::Value<QString>>();
    }
    throw Exception(node.Mark(), "Unexpected type");
}

static bool isEnum(const std::type_index &type)
{
    static const std::unordered_set<std::type_index> enums{typeid(Qt::KeyboardModifiers), typeid(InputDeviceTypes)};
    return enums.contains(type);
}

/**
 * @param arguments x and y, size must be 2
 */
static QPointF parseMouseInputActionPoint(const Node &node, const QStringList &arguments)
{
    if (arguments.length() != 2) {
        throw Exception(node.Mark(), "Invalid point (wrong argument count)");
    }

    bool ok1{};
    bool ok2{};
    const QPointF point(arguments[0].toDouble(&ok1), arguments[1].toDouble(&ok2));

    if (!ok1) {
        throw Exception(node.Mark(), "Invalid point (argument 1 is not a number)");
    }
    if (!ok2) {
        throw Exception(node.Mark(), "Invalid point (argument 2 is not a number)");
    }

    return point;
}

template<typename T>
struct convert<Range<T>>
{
    static bool decode(const Node &node, Range<T> &range)
    {
        const auto rangeRaw = node.as<QString>().replace(" ", "");
        if (rangeRaw.contains("-")) {
            const auto split = rangeRaw.split("-");
            range = Range<T>(split[0].toDouble(), split[1].toDouble());
        } else {
            range = Range<T>(rangeRaw.toDouble(), {});
        }

        return true;
    }
};

static std::shared_ptr<Condition> asVariableCondition(const Node &node, const VariableManager *variableManager)
{
    auto raw = node.as<QString>();
    bool negate{};
    if (raw.startsWith('!')) {
        raw = raw.mid(1);
        negate = true;
    }
    raw = raw.mid(1); // Remove $

    const auto firstSpace = raw.indexOf(' ');
    const auto secondSpace = raw.indexOf(' ', firstSpace + 1);

    const auto variableName = raw.left(firstSpace);
    const auto *variable = variableManager->getVariable(variableName);
    if (!variable) {
        // Variable type must be known in order to parse the right side of the condition
        throw Exception(node.Mark(), std::format("Variable {} does not exist.", variableName.toStdString()));
    }

    ComparisonOperator comparisonOperator;
    std::vector<InputActions::Value<std::any>> right;
    if (firstSpace == -1 && variable->type() == typeid(bool)) { // bool variable condition without operator
        comparisonOperator = ComparisonOperator::EqualTo;
        right.push_back(InputActions::Value<bool>(true));
    } else {
        static const std::unordered_map<QString, ComparisonOperator> operators = {
            {"==", ComparisonOperator::EqualTo},
            {"!=", ComparisonOperator::NotEqualTo},
            {">", ComparisonOperator::GreaterThan},
            {">=", ComparisonOperator::GreaterThanOrEqual},
            {"<", ComparisonOperator::LessThan},
            {"<=", ComparisonOperator::LessThanOrEqual},
            {"contains", ComparisonOperator::Contains},
            {"between", ComparisonOperator::Between},
            {"matches", ComparisonOperator::Regex},
            {"one_of", ComparisonOperator::OneOf},
        };
        const auto operatorRaw = raw.mid(firstSpace + 1, secondSpace - firstSpace - 1);
        if (!operators.contains(operatorRaw)) {
            throw Exception(node.Mark(), "Invalid operator");
        }
        comparisonOperator = operators.at(operatorRaw);

        const auto rightRaw = raw.mid(secondSpace + 1);
        const auto rightNode = YAML::Load(rightRaw.toStdString());

        if (!isEnum(variable->type()) && rightNode.IsSequence()) {
            for (const auto &child : rightNode) {
                right.push_back(asAny(child, variable->type()));
            }
        } else if (rightRaw.contains(';')) {
            const auto split = rightRaw.split(';');
            right.push_back(asAny(YAML::Load(split[0].toStdString()), variable->type()));
            right.push_back(asAny(YAML::Load(split[1].toStdString()), variable->type()));
        } else {
            right.push_back(asAny(rightNode, variable->type()));
        }
    }

    auto condition = std::make_shared<VariableCondition>(variableName, right, comparisonOperator);
    condition->setNegate(negate);
    return condition;
}

static std::shared_ptr<Condition> asCondition(const Node &node, const VariableManager *variableManager = {})
{
    if (!variableManager) {
        variableManager = g_variableManager.get();
    }

    static const auto isLegacy = [](const auto &node) {
        return node.IsMap() && (node["negate"] || node["window_class"] || node["window_state"]);
    };

    if (node.IsMap()) {
        std::optional<ConditionGroupMode> groupMode;
        Node groupChildren;
        if (node["all"]) {
            groupMode = ConditionGroupMode::All;
            groupChildren = Clone(node["all"]);
        } else if (node["any"]) {
            groupMode = ConditionGroupMode::Any;
            groupChildren = Clone(node["any"]);
        } else if (node["none"]) {
            groupMode = ConditionGroupMode::None;
            groupChildren = Clone(node["none"]);
        }
        if (groupMode) {
            auto group = std::make_shared<ConditionGroup>(*groupMode);
            for (const auto &child : groupChildren) {
                group->add(asCondition(child, variableManager));
            }
            return group;
        }

        if (isLegacy(node)) {
            auto group = std::make_shared<ConditionGroup>();
            const auto negate = node["negate"].as<QStringList>(QStringList());
            if (const auto &windowClassNode = node["window_class"]) {
                const auto value = InputActions::Value<QString>(windowClassNode.as<QString>());
                auto classGroup = std::make_shared<ConditionGroup>(ConditionGroupMode::Any);
                classGroup->add(std::make_shared<VariableCondition>("window_class", value, ComparisonOperator::Regex));
                classGroup->add(std::make_shared<VariableCondition>("window_name", value, ComparisonOperator::Regex));
                classGroup->setNegate(negate.contains("window_class"));
                group->add(classGroup);
            }
            if (const auto &windowStateNode = node["window_state"]) {
                const auto value = windowStateNode.as<QStringList>(QStringList());
                const auto trueValue = InputActions::Value<bool>(true);
                auto classGroup = std::make_shared<ConditionGroup>(ConditionGroupMode::Any);
                if (value.contains("fullscreen")) {
                    classGroup->add(std::make_shared<VariableCondition>("window_fullscreen", trueValue, ComparisonOperator::EqualTo));
                }
                if (value.contains("maximized")) {
                    classGroup->add(std::make_shared<VariableCondition>("window_maximized", trueValue, ComparisonOperator::EqualTo));
                }
                classGroup->setNegate(negate.contains("window_state"));
                group->add(classGroup);
            }
            return group;
        }
    }

    // Not in any group
    if (node.IsSequence()) {
        auto group = std::make_shared<ConditionGroup>(isLegacy(node[0]) ? ConditionGroupMode::Any : ConditionGroupMode::All);
        for (const auto &child : node) {
            group->add(asCondition(child, variableManager));
        }
        return group;
    }

    if (node.IsScalar()) {
        // Hack to load negated conditions without forcing users to quote the entire thing
        auto conditionNode = node;
        const auto tag = node.Tag();
        if (tag != "!" && tag.starts_with('!') && !conditionNode.as<QString>().startsWith("!$")) {
            conditionNode = QString("%1 %2").arg(QString::fromStdString(tag), node.as<QString>()).trimmed().toStdString();
        }

        const auto raw = conditionNode.as<QString>("");
        if (raw.startsWith("$") || raw.startsWith("!$")) {
            return asVariableCondition(conditionNode, variableManager);
        }
    }
    return {};
}

template<>
struct convert<std::shared_ptr<Condition>>
{
    static bool decode(const Node &node, std::shared_ptr<Condition> &condition)
    {
        condition = asCondition(node);
        return condition.get();
    }
};

template<typename T>
struct convert<std::set<T>>
{
    static bool decode(const Node &node, std::set<T> &set)
    {
        for (const auto &child : node) {
            set.insert(child.as<T>());
        }
        return true;
    }
};

template<>
struct convert<std::vector<std::unique_ptr<Trigger>>>
{
    static bool decode(const Node &node, std::vector<std::unique_ptr<Trigger>> &triggers)
    {
        for (const auto &triggerNode : node) {
            if (const auto &subTriggersNode = triggerNode["gestures"]) {
                // Trigger group
                for (const auto &subTriggerNode : subTriggersNode) {
                    // Trigger group
                    auto clonedNode = Clone(subTriggerNode);
                    for (auto it = triggerNode.begin(); it != triggerNode.end(); it++) {
                        auto name = it->first.as<QString>();
                        auto value = it->second;

                        if (name == "conditions") {
                            Node conditionsNode;
                            conditionsNode["all"] = Node();
                            conditionsNode["all"].push_back(triggerNode["conditions"]);
                            if (subTriggerNode["conditions"]) {
                                conditionsNode["all"].push_back(subTriggerNode["conditions"]);
                            }
                            clonedNode["conditions"] = conditionsNode;
                        } else if (name != "gestures") {
                            clonedNode[name.toStdString()] = value;
                        }
                    }

                    Node list;
                    list.push_back(clonedNode);
                    for (auto &trigger : list.as<std::vector<std::unique_ptr<Trigger>>>()) {
                        triggers.push_back(std::move(trigger));
                    }
                }
                continue;
            }

            triggers.push_back(triggerNode.as<std::unique_ptr<Trigger>>());
        }
        return true;
    }
};

template<typename T>
void loadMember(T &member, const Node &node)
{
    if (node) {
        member = node.as<T>();
    }
}
template<typename T>
void loadMember(std::optional<T> &member, const Node &node)
{
    if (node) {
        member = node.as<T>();
    }
}

template<typename T, typename TObject>
void loadSetter(TObject *object, void (TObject::*setter)(T), const Node &node)
{
    if (node) {
        (object->*setter)(node.as<T>());
    }
}

template<typename T, typename TObject>
void loadSetter(const std::unique_ptr<TObject> &object, void (TObject::*setter)(T), const Node &node)
{
    loadSetter(object.get(), setter, node);
}

template<typename T, typename TObject>
void loadSetter(const std::shared_ptr<TObject> &object, void (TObject::*setter)(T), const Node &node)
{
    loadSetter(object.get(), setter, node);
}

template<typename T, typename TObject>
void loadSetter(TObject &object, void (TObject::*setter)(T), const Node &node)
{
    loadSetter(&object, setter, node);
}

template<>
struct convert<std::unique_ptr<Trigger>>
{
    static bool decode(const Node &node, std::unique_ptr<Trigger> &trigger)
    {
        const auto type = node["type"].as<QString>();
        if (type == "circle") {
            trigger = std::make_unique<DirectionalMotionTrigger>(TriggerType::Circle, static_cast<TriggerDirection>(node["direction"].as<RotateDirection>()));
        } else if (type == "click") {
            trigger = std::make_unique<Trigger>(TriggerType::Click);
        } else if (type == "hold" || type == "press") {
            auto pressTrigger = new PressTrigger;
            loadSetter(pressTrigger, &PressTrigger::setInstant, node["instant"]);
            trigger.reset(pressTrigger);
        } else if (type == "hover") {
            trigger = std::make_unique<HoverTrigger>();
        } else if (type == "pinch") {
            trigger = std::make_unique<DirectionalMotionTrigger>(TriggerType::Pinch, static_cast<TriggerDirection>(node["direction"].as<PinchDirection>()));
        } else if (type == "rotate") {
            trigger = std::make_unique<DirectionalMotionTrigger>(TriggerType::Rotate, static_cast<TriggerDirection>(node["direction"].as<RotateDirection>()));
        } else if (type == "shortcut") {
            trigger = std::make_unique<KeyboardShortcutTrigger>(node["shortcut"].as<KeyboardShortcut>());
        } else if (type == "stroke") {
            trigger = std::make_unique<StrokeTrigger>(asSequence(node["strokes"]).as<std::vector<Stroke>>());
        } else if (type == "swipe") {
            trigger = std::make_unique<DirectionalMotionTrigger>(TriggerType::Swipe, static_cast<TriggerDirection>(node["direction"].as<SwipeDirection>()));
        } else if (type == "tap") {
            trigger = std::make_unique<Trigger>(TriggerType::Tap);
        } else if (type == "wheel") {
            trigger = std::make_unique<WheelTrigger>(static_cast<TriggerDirection>(node["direction"].as<SwipeDirection>()));
        } else {
            throw Exception(node.Mark(), "Invalid trigger type");
        }

        loadSetter(trigger, &Trigger::setBlockEvents, node["block_events"]);
        loadSetter(trigger, &Trigger::setClearModifiers, node["clear_modifiers"]);
        loadSetter(trigger, &Trigger::setEndCondition, node["end_conditions"]);
        loadSetter(trigger, &Trigger::setId, node["id"]);
        loadSetter(trigger, &Trigger::setMouseButtons, node["mouse_buttons"]);
        loadSetter(trigger, &Trigger::setMouseButtonsExactOrder, node["mouse_buttons_exact_order"]);
        loadSetter(trigger, &Trigger::setResumeTimeout, node["resume_timeout"]);
        loadSetter(trigger, &Trigger::setSetLastTrigger, node["set_last_trigger"]);
        loadSetter(trigger, &Trigger::setThreshold, node["threshold"]);
        if (auto *motionTrigger = dynamic_cast<MotionTrigger *>(trigger.get())) {
            loadSetter(motionTrigger, &MotionTrigger::setLockPointer, node["lock_pointer"]);
            loadSetter(motionTrigger, &MotionTrigger::setSpeed, node["speed"]);
        }

        auto conditionGroup = std::make_shared<ConditionGroup>();
        if (const auto &fingersNode = node["fingers"]) {
            auto range = fingersNode.as<Range<qreal>>();
            if (!range.max()) {
                conditionGroup->add(std::make_shared<VariableCondition>(BuiltinVariables::Fingers,
                                                                        InputActions::Value<qreal>(range.min().value()),
                                                                        ComparisonOperator::EqualTo));
            } else {
                conditionGroup->add(std::make_shared<VariableCondition>(BuiltinVariables::Fingers,
                                                                        std::vector<InputActions::Value<std::any>>{
                                                                            InputActions::Value<qreal>(range.min().value()),
                                                                            InputActions::Value<qreal>(range.max().value())},
                                                                        ComparisonOperator::Between));
            }
        }
        if (const auto &modifiersNode = node["keyboard_modifiers"]) {
            std::optional<Qt::KeyboardModifiers> modifiers;
            if (modifiersNode.IsSequence()) {
                modifiers = modifiersNode.as<Qt::KeyboardModifiers>();
            } else {
                const auto modifierMatchingMode = modifiersNode.as<QString>();
                if (modifierMatchingMode == "none") {
                    modifiers = Qt::KeyboardModifier::NoModifier;
                } else if (modifierMatchingMode != "any") {
                    throw Exception(node.Mark(), "Invalid keyboard modifier");
                }
            }

            if (modifiers) {
                conditionGroup->add(std::make_shared<VariableCondition>(BuiltinVariables::KeyboardModifiers,
                                                                        InputActions::Value<Qt::KeyboardModifiers>(modifiers.value()),
                                                                        ComparisonOperator::EqualTo));
            }
        }
        if (const auto &conditionsNode = node["conditions"]) {
            conditionGroup->add(conditionsNode.as<std::shared_ptr<Condition>>());
        }
        trigger->setActivationCondition(conditionGroup);

        const auto accelerated = node["accelerated"].as<bool>(false);
        for (const auto &actionNode : node["actions"]) {
            auto action = actionNode.as<std::unique_ptr<TriggerAction>>();
            action->setAccelerated(accelerated);
            trigger->addAction(std::move(action));
        }

        return true;
    }
};

template<>
struct convert<std::unique_ptr<TriggerAction>>
{
    static bool decode(const Node &node, std::unique_ptr<TriggerAction> &value)
    {
        value = std::make_unique<TriggerAction>(node.as<std::shared_ptr<Action>>());

        loadSetter(value, &TriggerAction::setConflicting, node["conflicting"]);
        loadSetter(value, &TriggerAction::setInterval, node["interval"]);
        loadSetter(value, &TriggerAction::setOn, node["on"]);
        loadSetter(value, &TriggerAction::setThreshold, node["threshold"]);

        if (value->on() == On::Begin && value->threshold() && (value->threshold()->min() || value->threshold()->max())) {
            throw Exception(node.Mark(), "Begin actions can't have thresholds");
        }

        return true;
    }
};

template<>
struct convert<std::shared_ptr<Action>>
{
    static bool decode(const Node &node, std::shared_ptr<Action> &value)
    {
        if (const auto &commandNode = node["command"]) {
            auto action = std::make_shared<CommandAction>(commandNode.as<InputActions::Value<QString>>());
            loadSetter(action, &CommandAction::setWait, node["wait"]);
            value = action;
        } else if (const auto &inputNode = node["input"]) {
            auto action = std::make_shared<InputAction>(inputNode.as<std::vector<InputAction::Item>>());
            loadSetter(action, &InputAction::setDelay, node["delay"]);
            value = action;
        } else if (const auto &plasmaShortcutNode = node["plasma_shortcut"]) {
            const auto split = plasmaShortcutNode.as<QString>().split(",");
            if (split.length() != 2) {
                throw Exception(node.Mark(), "Invalid Plasma shortcut format");
            }
            value = std::make_shared<PlasmaGlobalShortcutAction>(split[0], split[1]);
        } else if (const auto &sleepActionNode = node["sleep"]) {
            value = std::make_shared<SleepAction>(sleepActionNode.as<std::chrono::milliseconds>());
        } else if (const auto &oneNode = node["one"]) {
            value = std::make_shared<ActionGroup>(oneNode.as<std::vector<std::shared_ptr<Action>>>(), ActionGroup::ExecutionMode::First);
        } else {
            throw Exception(node.Mark(), "Action has no valid action property");
        }

        loadSetter(value, &Action::setCondition, node["conditions"]);
        loadSetter(value, &Action::setExecutionLimit, node["limit"]);
        loadSetter(value, &Action::setId, node["id"]);

        return true;
    }
};

template<>
struct convert<ActionInterval>
{
    static bool decode(const Node &node, ActionInterval &interval)
    {
        const auto intervalRaw = node.as<QString>();
        if (intervalRaw == "+") {
            interval.setDirection(IntervalDirection::Positive);
            return true;
        } else if (intervalRaw == "-") {
            interval.setDirection(IntervalDirection::Negative);
            return true;
        }

        if (const auto value = node.as<qreal>()) {
            interval.setValue(value);
            interval.setDirection(value < 0 ? IntervalDirection::Negative : IntervalDirection::Positive);
        }

        return true;
    }
};

static void decodeTriggerHandler(const Node &node, TriggerHandler *handler)
{
    const auto &triggersNode = node["gestures"];
    if (!triggersNode.IsDefined()) {
        throw Exception(node.Mark(), "No gestures specified");
    }
    for (auto &trigger : triggersNode.as<std::vector<std::unique_ptr<Trigger>>>()) {
        handler->addTrigger(std::move(trigger));
    }
    if (const auto &timeDeltaNode = node["__time_delta"]) {
        handler->setTimedTriggerUpdateDelta(timeDeltaNode.as<uint32_t>());
    }
}

static void decodeMotionTriggerHandler(const Node &node, TriggerHandler *handler)
{
    decodeTriggerHandler(node, handler);

    auto *motionHandler = dynamic_cast<MotionTriggerHandler *>(handler);
    if (const auto &speedNode = node["speed"]) {
        loadSetter(motionHandler, &MotionTriggerHandler::setInputEventsToSample, speedNode["events"]);
        if (const auto &thresholdNode = speedNode["swipe_threshold"]) {
            motionHandler->setSpeedThreshold(TriggerType::Swipe, thresholdNode.as<qreal>());
        }
    }
}

static void decodeMultiTouchMotionTriggerHandler(const Node &node, TriggerHandler *handler)
{
    decodeMotionTriggerHandler(node, handler);

    auto *multiTouchMotionHandler = dynamic_cast<MultiTouchMotionTriggerHandler *>(handler);
    if (const auto &speedNode = node["speed"]) {
        if (const auto &thresholdNode = speedNode["pinch_in_threshold"]) {
            multiTouchMotionHandler->setSpeedThreshold(TriggerType::Pinch, thresholdNode.as<qreal>(), static_cast<TriggerDirection>(PinchDirection::In));
        }
        if (const auto &thresholdNode = speedNode["pinch_out_threshold"]) {
            multiTouchMotionHandler->setSpeedThreshold(TriggerType::Pinch, thresholdNode.as<qreal>(), static_cast<TriggerDirection>(PinchDirection::Out));
        }
        if (const auto &thresholdNode = speedNode["rotate_threshold"]) {
            multiTouchMotionHandler->setSpeedThreshold(TriggerType::Rotate, thresholdNode.as<qreal>());
        }
    }
}

template<>
struct convert<std::unique_ptr<KeyboardTriggerHandler>>
{
    static bool decode(const Node &node, std::unique_ptr<KeyboardTriggerHandler> &handler)
    {
        handler = std::make_unique<KeyboardTriggerHandler>();
        decodeTriggerHandler(node, handler.get());
        return true;
    }
};

template<>
struct convert<std::unique_ptr<MouseTriggerHandler>>
{
    static bool decode(const Node &node, std::unique_ptr<MouseTriggerHandler> &handler)
    {
        auto *mouseTriggerHandler = new MouseTriggerHandler;
        handler.reset(mouseTriggerHandler);
        decodeMotionTriggerHandler(node, handler.get());
        return true;
    }
};

template<>
struct convert<std::unique_ptr<PointerTriggerHandler>>
{
    static bool decode(const Node &node, std::unique_ptr<PointerTriggerHandler> &handler)
    {
        handler = std::make_unique<PointerTriggerHandler>();
        decodeTriggerHandler(node, handler.get());
        return true;
    }
};

std::unique_ptr<TouchpadTriggerHandler> asTouchpadTriggerHandler(const Node &node, InputDevice *device)
{
    auto handler = std::make_unique<TouchpadTriggerHandler>(device);
    decodeMultiTouchMotionTriggerHandler(node, handler.get());
    loadSetter(static_cast<MotionTriggerHandler *>(handler.get()), &MotionTriggerHandler::setSwipeDeltaMultiplier, node["delta_multiplier"]);
    return handler;
}

std::unique_ptr<TouchscreenTriggerHandler> asTouchscreenTriggerHandler(const Node &node, InputDevice *device)
{
    auto handler = std::make_unique<TouchscreenTriggerHandler>(device);
    decodeMultiTouchMotionTriggerHandler(node, handler.get());
    return handler;
}

#define ENUM_DECODER(type, error, map)                                                                    \
    template<>                                                                                            \
    struct convert<type>                                                                                  \
    {                                                                                                     \
        static bool decode(const Node &node, type &value)                                                 \
        {                                                                                                 \
            const auto raw = node.as<QString>();                                                          \
            if (!map.contains(raw)) {                                                                     \
                throw Exception(node.Mark(), QString("Invalid %1 ('%2')").arg(error, raw).toStdString()); \
            }                                                                                             \
            value = map.at(raw);                                                                          \
            return true;                                                                                  \
        }                                                                                                 \
    };

#define FLAGS_DECODER(type, error, map)                                                                       \
    template<>                                                                                                \
    struct convert<type>                                                                                      \
    {                                                                                                         \
        static bool decode(const Node &node, type &values)                                                    \
        {                                                                                                     \
            values = {0};                                                                                     \
            for (const auto &raw : node.as<QStringList>()) {                                                  \
                if (!map.contains(raw)) {                                                                     \
                    throw Exception(node.Mark(), QString("Invalid %1 ('%2')").arg(error, raw).toStdString()); \
                }                                                                                             \
                values |= map.at(raw);                                                                        \
            }                                                                                                 \
            return true;                                                                                      \
        }                                                                                                     \
    };

ENUM_DECODER(On, "action event (on)",
             (std::unordered_map<QString, On>{
                 {"begin", On::Begin},
                 {"cancel", On::Cancel},
                 {"end", On::End},
                 {"end_cancel", On::EndCancel},
                 {"tick", On::Tick},
                 {"update", On::Update},
             }))
ENUM_DECODER(CursorShape, "cursor shape", CURSOR_SHAPES)
ENUM_DECODER(PinchDirection, "pinch direction",
             (std::unordered_map<QString, PinchDirection>{
                 {"in", PinchDirection::In},
                 {"out", PinchDirection::Out},
                 {"any", PinchDirection::Any},
             }))
ENUM_DECODER(RotateDirection, "rotate direction",
             (std::unordered_map<QString, RotateDirection>{
                 {"clockwise", RotateDirection::Clockwise},
                 {"counterclockwise", RotateDirection::Counterclockwise},
                 {"any", RotateDirection::Any},
             }))
ENUM_DECODER(SwipeDirection, "swipe direction",
             (std::unordered_map<QString, SwipeDirection>{
                 {"left", SwipeDirection::Left},
                 {"right", SwipeDirection::Right},
                 {"up", SwipeDirection::Up},
                 {"down", SwipeDirection::Down},
                 {"up_down", SwipeDirection::UpDown},
                 {"left_right", SwipeDirection::LeftRight},
                 {"any", SwipeDirection::Any},
             }))
ENUM_DECODER(TriggerSpeed, "trigger speed",
             (std::unordered_map<QString, TriggerSpeed>{
                 {"fast", TriggerSpeed::Fast},
                 {"slow", TriggerSpeed::Slow},
                 {"any", TriggerSpeed::Any},
             }))

FLAGS_DECODER(Qt::KeyboardModifiers, "keyboard modifier",
              (std::unordered_map<QString, Qt::KeyboardModifier>{
                  {"alt", Qt::KeyboardModifier::AltModifier},
                  {"ctrl", Qt::KeyboardModifier::ControlModifier},
                  {"meta", Qt::KeyboardModifier::MetaModifier},
                  {"shift", Qt::KeyboardModifier::ShiftModifier},
              }))
FLAGS_DECODER(InputDeviceTypes, "input device type",
              (std::unordered_map<QString, InputDeviceType>{
                  {"keyboard", InputDeviceType::Keyboard},
                  {"mouse", InputDeviceType::Mouse},
                  {"touchpad", InputDeviceType::Touchpad},
                  {"touchscreen", InputDeviceType::Touchscreen},
              }))

template<>
struct convert<std::vector<InputAction::Item>>
{
    static bool decode(const Node &node, std::vector<InputAction::Item> &value)
    {
        for (const auto &device : node) {
            if (device["keyboard"].IsDefined()) {
                for (const auto &actionNode : device["keyboard"]) {
                    if (actionNode.IsMap() && actionNode["text"].IsDefined()) {
                        value.push_back({
                            .keyboardText = actionNode["text"].as<InputActions::Value<QString>>(),
                        });
                    } else {
                        const auto actionRaw = actionNode.as<QString>().toUpper();
                        if (actionRaw.startsWith("+") || actionRaw.startsWith("-")) {
                            const auto keyRaw = actionRaw.mid(1);
                            const auto key = KeyboardKey::fromString(keyRaw);
                            if (!key) {
                                throw Exception(node.Mark(), ("Invalid keyboard key ('" + keyRaw + "')").toStdString());
                            }

                            if (actionRaw[0] == '+') {
                                value.push_back({
                                    .keyboardPress = key.value(),
                                });
                            } else {
                                value.push_back({
                                    .keyboardRelease = key.value(),
                                });
                            }
                        } else {
                            std::vector<KeyboardKey> keys;
                            for (const auto &keyRaw : actionRaw.split("+")) {
                                const auto key = KeyboardKey::fromString(keyRaw);
                                if (!key) {
                                    throw Exception(node.Mark(), ("Invalid keyboard key ('" + keyRaw + "')").toStdString());
                                }
                                keys.push_back(key.value());
                            }

                            for (const auto key : keys) {
                                value.push_back({
                                    .keyboardPress = key,
                                });
                            }
                            std::reverse(keys.begin(), keys.end());
                            for (const auto key : keys) {
                                value.push_back({
                                    .keyboardRelease = key,
                                });
                            }
                        }
                    }
                }
            } else if (device["mouse"].IsDefined()) {
                for (auto &actionRaw : device["mouse"].as<QStringList>()) {
                    const auto split = actionRaw.split(' ');
                    const auto action = split[0].toUpper();
                    const auto arguments = split.mid(1);

                    if (action.startsWith("+") || action.startsWith("-")) {
                        const auto buttonRaw = action.mid(1);
                        const auto button = MouseButton::fromString(buttonRaw);
                        if (!button) {
                            throw Exception(node.Mark(), ("Invalid mouse button ('" + buttonRaw + "')").toStdString());
                        }

                        if (action[0] == '+') {
                            value.push_back({
                                .mousePress = button.value(),
                            });
                        } else {
                            value.push_back({
                                .mouseRelease = button.value(),
                            });
                        }
                    } else if (action.startsWith("MOVE_BY_DELTA")) {
                        qreal multiplier = 1;
                        if (arguments.size() > 0) {
                            bool ok{};
                            multiplier = arguments.at(0).toDouble(&ok);
                            if (!ok) {
                                throw Exception(node.Mark(), "move_by_delta multiplier is not a number");
                            }
                        }

                        value.push_back({
                            .mouseMoveRelativeByDelta = multiplier,
                        });
                    } else if (action.startsWith("MOVE_BY")) {
                        value.push_back({
                            .mouseMoveRelative = parseMouseInputActionPoint(node, arguments),
                        });
                    } else if (action.startsWith("MOVE_TO")) {
                        value.push_back({
                            .mouseMoveAbsolute = parseMouseInputActionPoint(node, arguments),
                        });
                    } else if (action.startsWith("WHEEL")) {
                        value.push_back({
                            .mouseAxis = parseMouseInputActionPoint(node, arguments),
                        });
                    } else {
                        std::vector<MouseButton> buttons;
                        for (const auto &buttonRaw : action.split("+")) {
                            const auto button = MouseButton::fromString(buttonRaw);
                            if (!button) {
                                throw Exception(node.Mark(), ("Invalid mouse button ('" + buttonRaw + "')").toStdString());
                            }
                            buttons.push_back(button.value());
                        }

                        for (const auto button : buttons) {
                            value.push_back({
                                .mousePress = button,
                            });
                        }
                        std::reverse(buttons.begin(), buttons.end());
                        for (const auto button : buttons) {
                            value.push_back({
                                .mouseRelease = button,
                            });
                        }
                    }
                }
            }
        }

        return true;
    }
};

template<>
struct convert<KeyboardKey>
{
    static bool decode(const Node &node, KeyboardKey &value)
    {
        const auto raw = node.as<QString>();
        if (const auto key = KeyboardKey::fromString(raw)) {
            value = key.value();
            return true;
        }

        throw Exception(node.Mark(), ("Invalid keyboard key ('" + raw + "')").toStdString());
    }
};

template<>
struct convert<MouseButton>
{
    static bool decode(const Node &node, MouseButton &value)
    {
        const auto raw = node.as<QString>();
        if (const auto key = MouseButton::fromString(raw)) {
            value = key.value();
            return true;
        }

        throw Exception(node.Mark(), ("Invalid mouse button ('" + raw + "')").toStdString());
    }
};

template<>
struct convert<KeyboardShortcut>
{
    static bool decode(const Node &node, KeyboardShortcut &value)
    {
        loadMember(value.keys, node);
        return true;
    }
};

template<>
struct convert<std::vector<InputDeviceRule>>
{
    static bool decode(const Node &node, std::vector<InputDeviceRule> &value)
    {
        for (const auto &ruleNode : node["device_rules"]) {
            InputDeviceRule rule;
            if (const auto conditionsNode = ruleNode["conditions"]) {
                rule.setCondition(asCondition(conditionsNode, &g_inputBackend->deviceRulesVariableManager()));
            }
            loadSetter(rule, &InputDeviceRule::setProperties, ruleNode);
            value.push_back(std::move(rule));
        }

        // Legacy
        if (const auto &mouseNode = node["mouse"]) {
            InputDeviceRule rule;
            rule.setCondition(std::make_shared<VariableCondition>("types",
                                                                  InputActions::Value<InputDeviceTypes>(InputDeviceType::Mouse),
                                                                  ComparisonOperator::Contains));
            loadSetter(rule.properties(), &InputDeviceProperties::setMouseMotionTimeout, mouseNode["motion_timeout"]);
            loadSetter(rule.properties(), &InputDeviceProperties::setMousePressTimeout, mouseNode["press_timeout"]);
            loadSetter(rule.properties(), &InputDeviceProperties::setMouseUnblockButtonsOnTimeout, mouseNode["unblock_buttons_on_timeout"]);
            value.push_back(std::move(rule));
        }

        if (const auto &touchpadNode = node["touchpad"]) {
            if (const auto &clickTimeoutNode = touchpadNode["click_timeout"]) {
                InputDeviceRule rule;
                rule.setCondition(std::make_shared<VariableCondition>("types",
                                                                      InputActions::Value<InputDeviceTypes>(InputDeviceType::Touchpad),
                                                                      ComparisonOperator::Contains));
                loadSetter(rule.properties(), &InputDeviceProperties::setTouchpadClickTimeout, clickTimeoutNode);
                value.push_back(std::move(rule));
            }

            if (const auto &devicesNode = touchpadNode["devices"]) {
                for (auto it = devicesNode.begin(); it != devicesNode.end(); ++it) {
                    InputDeviceRule rule;
                    rule.setCondition(std::make_shared<VariableCondition>("name", InputActions::Value(it->first.as<QString>()), ComparisonOperator::EqualTo));
                    loadSetter(rule, &InputDeviceRule::setProperties, it->second);
                    value.push_back(std::move(rule));
                }
            }
        }

        return true;
    }
};

template<>
struct convert<InputDeviceProperties>
{
    static bool decode(const Node &node, InputDeviceProperties &value)
    {
        loadSetter(value, &InputDeviceProperties::setMultiTouch, node["__multiTouch"]);
        loadSetter(value, &InputDeviceProperties::setGrab, node["grab"]);
        loadSetter(value, &InputDeviceProperties::setHandleLibevdevEvents, node["handle_libevdev_events"]);
        loadSetter(value, &InputDeviceProperties::setIgnore, node["ignore"]);
        loadSetter(value, &InputDeviceProperties::setMouseMotionTimeout, node["motion_timeout"]);
        loadSetter(value, &InputDeviceProperties::setMousePressTimeout, node["press_timeout"]);
        loadSetter(value, &InputDeviceProperties::setMouseUnblockButtonsOnTimeout, node["unblock_buttons_on_timeout"]);
        loadSetter(value, &InputDeviceProperties::setTouchpadButtonPad, node["buttonpad"]);
        loadSetter(value, &InputDeviceProperties::setTouchpadClickTimeout, node["click_timeout"]);

        if (const auto &pressureRangesNode = node["pressure_ranges"]) {
            loadSetter(value, &InputDeviceProperties::setFingerPressure, pressureRangesNode["finger"]);
            loadSetter(value, &InputDeviceProperties::setThumbPressure, pressureRangesNode["thumb"]);
            loadSetter(value, &InputDeviceProperties::setPalmPressure, pressureRangesNode["palm"]);
        }
        return true;
    }
};

template<>
struct convert<QPointF>
{
    static bool decode(const Node &node, QPointF &point)
    {
        const auto raw = node.as<QString>().split(",");
        if (raw.size() != 2) {
            throw Exception(node.Mark(), "Invalid point");
        }

        bool okX, okY = false;
        const auto x = raw[0].toDouble(&okX);
        const auto y = raw[1].toDouble(&okY);
        if (!okX || !okY) {
            throw Exception(node.Mark(), "Failed to parse number");
        }

        point = {x, y};
        return true;
    }
};

template<>
struct convert<QString>
{
    static bool decode(const Node &node, QString &s)
    {
        s = QString::fromStdString(node.as<std::string>());
        return true;
    }
};

template<>
struct convert<QStringList>
{
    static bool decode(const Node &node, QStringList &list)
    {
        for (const auto &s : node.as<std::vector<QString>>()) {
            list << s;
        }
        return true;
    }
};

template<>
struct convert<QRegularExpression>
{
    static bool decode(const Node &node, QRegularExpression &regex)
    {
        regex = QRegularExpression(node.as<QString>());
        return true;
    }
};

template<typename T>
struct convert<InputActions::Value<T>>
{
    static bool decode(const Node &node, InputActions::Value<T> &value)
    {
        if (node.IsMap()) {
            if (const auto &commandNode = node["command"]) {
                value = InputActions::Value<T>::command(commandNode.as<InputActions::Value<QString>>());
            }
        } else {
            const auto raw = node.as<QString>();
            const auto variableName = raw.mid(1); // remove $
            if (g_variableManager->hasVariable(variableName)) {
                value = InputActions::Value<T>::variable(variableName);
            } else {
                value = InputActions::Value<T>(node.as<T>());
            }
        }
        return true;
    }
};

template<>
struct convert<Stroke>
{
    static bool decode(const Node &node, Stroke &stroke)
    {
        const auto bytes = QByteArray::fromBase64(node.as<QString>().toUtf8());
        if (bytes.size() % 4 != 0) {
            throw Exception(node.Mark(), "Invalid stroke");
        }
        std::vector<Point> points;
        for (qsizetype i = 0; i < bytes.size(); i += 4) {
            points.push_back({
                .x = bytes[i] / 100.0,
                .y = bytes[i + 1] / 100.0,
                .t = bytes[i + 2] / 100.0,
                .alpha = bytes[i + 3] / 100.0,
            });
        }
        stroke = Stroke(points);

        return true;
    }
};

template<>
struct convert<std::chrono::milliseconds>
{
    static bool decode(const Node &node, std::chrono::milliseconds &value)
    {
        value = std::chrono::milliseconds(node.as<uint64_t>());
        return true;
    }
};

}
