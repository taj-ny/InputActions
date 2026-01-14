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

#include <libinputactions/config/Config.h>
#include <libinputactions/config/Node.h>
#include "core.h"
#include "containers.h"
#include "utils.h"
#include <QPointF>
#include <QRegularExpression>
#include <QString>

#include <libinputactions/Value.h>
#include <libinputactions/actions/ActionGroup.h>
#include <libinputactions/actions/CommandAction.h>
#include <libinputactions/actions/InputAction.h>
#include <libinputactions/actions/PlasmaGlobalShortcutAction.h>
#include <libinputactions/actions/SleepAction.h>
#include <libinputactions/actions/TriggerAction.h>
#include <libinputactions/conditions/ConditionGroup.h>
#include <libinputactions/conditions/LazyCondition.h>
#include <libinputactions/conditions/VariableCondition.h>
#include <libinputactions/handlers/KeyboardTriggerHandler.h>
#include <libinputactions/handlers/MouseTriggerHandler.h>
#include <libinputactions/handlers/PointerTriggerHandler.h>
#include <libinputactions/handlers/TouchpadTriggerHandler.h>
#include <libinputactions/handlers/TouchscreenTriggerHandler.h>
#include <libinputactions/input/InputDeviceRule.h>
#include <libinputactions/input/KeyboardKey.h>
#include <libinputactions/interfaces/CursorShapeProvider.h>
#include <libinputactions/triggers/HoverTrigger.h>
#include <libinputactions/triggers/KeyboardShortcutTrigger.h>
#include <libinputactions/triggers/PressTrigger.h>
#include <libinputactions/triggers/StrokeTrigger.h>
#include <libinputactions/triggers/WheelTrigger.h>
#include <libinputactions/variables/Variable.h>
#include <libinputactions/variables/VariableManager.h>

namespace InputActions
{

// https://invent.kde.org/plasma/kwin/-/blob/cc4d99ae/src/mousebuttons.cpp#L14
static const std::unordered_map<QString, uint32_t> MOUSE = {
    {"LEFT", BTN_LEFT},
    {"MIDDLE", BTN_MIDDLE},
    {"RIGHT", BTN_RIGHT},

    // Those 5 buttons are supposed to be like this (I think)
    {"BACK", BTN_SIDE},
    {"FORWARD", BTN_EXTRA},
    {"TASK", BTN_FORWARD},
    {"SIDE", BTN_BACK},
    {"EXTRA", BTN_TASK},

    {"EXTRA6", 0x118},
    {"EXTRA7", 0x119},
    {"EXTRA8", 0x11a},
    {"EXTRA9", 0x11b},
    {"EXTRA10", 0x11c},
    {"EXTRA11", 0x11d},
    {"EXTRA12", 0x11e},
    {"EXTRA13", 0x11f},
};

class ConfigParser;

inline KeyboardKey parseKeyboardKey(const Node *node, const QString &raw)
{
    if (const auto key = KeyboardKey::fromString(raw.toUpper())) {
        return key.value();
    }

    throw ConfigParserException(node, QString("Invalid keyboard key '%1'.").arg(raw));
}

inline uint32_t parseMouseButton(const Node *node, const QString &raw)
{
    if (MOUSE.contains(raw)) {
        return MOUSE.at(raw);
    }

    throw ConfigParserException(node, QString("Invalid mouse button '%1'.").arg(raw));
}

static Value<std::any> asAny(const Node *node, const std::type_index &type)
{
    if (type == typeid(bool)) {
        return node->as<Value<bool>>();
    } else if (type == typeid(CursorShape)) {
        return node->as<Value<CursorShape>>();
    } else if (type == typeid(Qt::KeyboardModifiers)) {
        return Value<Qt::KeyboardModifiers>(node->asSequence().as<Qt::KeyboardModifiers>()); // TODO
    } else if (type == typeid(InputDeviceTypes)) {
        return Value<InputDeviceTypes>(node->asSequence().as<InputDeviceTypes>()); // TODO
    } else if (type == typeid(qreal)) {
        return node->as<Value<qreal>>();
    } else if (type == typeid(QPointF)) {
        return node->as<Value<QPointF>>();
    } else if (type == typeid(QString)) {
        return node->as<Value<QString>>();
    }
    throw ConfigParserException(node, "Unexpected type");
}

static bool isEnum(const std::type_index &type)
{
    static const std::unordered_set<std::type_index> enums{typeid(Qt::KeyboardModifiers), typeid(InputDeviceTypes)};
    return enums.contains(type);
}

/**
 * @param arguments x and y, size must be 2
 */
static QPointF parseMouseInputActionPoint(const Node *node, const QStringList &arguments)
{
    if (arguments.length() != 2) {
        throw ConfigParserException(node, "Invalid point (wrong argument count)");
    }

    bool ok1{};
    bool ok2{};
    const QPointF point(arguments[0].toDouble(&ok1), arguments[1].toDouble(&ok2));

    if (!ok1) {
        throw ConfigParserException(node, "Invalid point (argument 1 is not a number)");
    }
    if (!ok2) {
        throw ConfigParserException(node, "Invalid point (argument 2 is not a number)");
    }

    return point;
}

static std::shared_ptr<Condition> parseVariableCondition(const Node *node, std::optional<ConditionEvaluationArguments> constructionArguments = {})
{
    auto raw = node->as<QString>();
    bool negate{};
    if (raw.startsWith('!')) {
        raw = raw.mid(1);
        negate = true;
    }
    raw = raw.mid(1); // Remove $

    const auto firstSpace = raw.indexOf(' ');
    const auto secondSpace = raw.indexOf(' ', firstSpace + 1);

    const auto variableName = raw.left(firstSpace);
    if (!constructionArguments) {
        // Variable type must be known in order to parse the right side of the condition
        return std::make_shared<LazyCondition>([variableName, node = node->clone()](const ConditionEvaluationArguments &arguments) {
            if (!arguments.variableManager->getVariable(variableName)) {
                throw ConfigParserException(&node, QString("Variable %1 does not exist.").arg(variableName.toStdString().c_str()));
            }
            return parseVariableCondition(&node, arguments);
        });
    }

    const auto variable = constructionArguments->variableManager->getVariable(variableName);
    ComparisonOperator comparisonOperator;
    std::vector<Value<std::any>> right;
    if (firstSpace == -1 && variable->type() == typeid(bool)) { // bool variable condition without operator
        comparisonOperator = ComparisonOperator::EqualTo;
        right.push_back(Value<bool>(true));
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
            throw ConfigParserException(node, "Invalid operator");
        }
        comparisonOperator = operators.at(operatorRaw);

        const auto rightRaw = raw.mid(secondSpace + 1);
        Node rightNode(rightRaw, node);

        if (!isEnum(variable->type()) && rightNode.raw()->IsSequence()) {
            for (auto child : rightNode.sequenceChildren()) {
                child->setPosition(node->line(), node->column());
                right.push_back(asAny(child.get(), variable->type()));
            }
        } else if (rightRaw.contains(';')) {
            const auto split = rightRaw.split(';');
            Node tmp1(split[0], node);
            Node tmp2(split[1], node);

            right.push_back(asAny(&tmp1, variable->type()));
            right.push_back(asAny(&tmp2, variable->type()));
        } else {
            right.push_back(asAny(&rightNode, variable->type()));
        }
    }

    auto condition = std::make_shared<VariableCondition>(variableName, right, comparisonOperator);
    condition->setNegate(negate);
    return condition;
}

template<>
inline void NodeParser<std::shared_ptr<Action>>::parse(const Node *node, std::shared_ptr<Action> &result)
{
    if (const auto commandNode = node->at("command")) {
        auto action = std::make_shared<CommandAction>(commandNode->as<Value<QString>>());
        loadSetter(action, &CommandAction::setWait, node->at("wait").get());
        result = action;
    } else if (const auto inputNode = node->at("input")) {
        auto action = std::make_shared<InputAction>(inputNode->as<std::vector<InputAction::Item>>());
        loadSetter(action, &InputAction::setDelay, node->at("delay").get());
        result = action;
    } else if (const auto plasmaShortcutNode = node->at("plasma_shortcut")) {
        const auto split = plasmaShortcutNode->as<QString>().split(",");
        if (split.length() != 2) {
            throw ConfigParserException(node, "Invalid Plasma shortcut format");
        }
        result = std::make_shared<PlasmaGlobalShortcutAction>(split[0], split[1]);
    } else if (const auto sleepActionNode = node->at("sleep")) {
        result = std::make_shared<SleepAction>(sleepActionNode->as<std::chrono::milliseconds>());
    } else if (const auto oneNode = node->at("one")) {
        result = std::make_shared<ActionGroup>(oneNode->as<std::vector<std::shared_ptr<Action>>>(), ActionGroup::ExecutionMode::First);
    } else {
        throw ConfigParserException(node, "Action is missing a required property that determines its type.");
    }

    loadSetter(result, &Action::setCondition, node->at("conditions").get());
    loadSetter(result, &Action::setExecutionLimit, node->at("limit").get());
    loadSetter(result, &Action::setId, node->at("id").get());
}

template<>
void NodeParser<ActionInterval>::parse(const Node *node, ActionInterval &result)
{
    const auto intervalRaw = node->as<QString>();
    if (intervalRaw == "+") {
        result.setDirection(IntervalDirection::Positive);
        return;
    } else if (intervalRaw == "-") {
        result.setDirection(IntervalDirection::Negative);
        return;
    }

    if (const auto value = node->as<qreal>()) {
        result.setValue(value);
        result.setDirection(value < 0 ? IntervalDirection::Negative : IntervalDirection::Positive);
    }
}

template<>
struct NodeParser<std::shared_ptr<Condition>>
{
    static bool isLegacy(const YAML::Node &node) { return node.IsMap() && (node["negate"] || node["window_class"] || node["window_state"]); }

    static void parse(const Node *node, std::shared_ptr<Condition> &condition)
    {
        if (node->raw()->IsMap()) {
            std::optional<ConditionGroupMode> groupMode;
            std::shared_ptr<const Node> groupChildren;
            if (const auto allNode = node->at("all")) {
                groupMode = ConditionGroupMode::All;
                groupChildren = allNode;
            } else if (const auto anyNode = node->at("any")) {
                groupMode = ConditionGroupMode::Any;
                groupChildren = anyNode;
            } else if (const auto noneNode = node->at("node")) {
                groupMode = ConditionGroupMode::None;
                groupChildren = noneNode;
            }
            if (groupMode) {
                auto group = std::make_shared<ConditionGroup>(*groupMode);
                for (const auto &child : groupChildren->sequenceChildren()) {
                    group->add(child->as<std::shared_ptr<Condition>>());
                }
                condition = group;
                return;
            }

            if (isLegacy(*node->raw())) {
                g_config->addIssue(node, ConfigIssueSeverity::Deprecation, "This method of defining conditions is deprecated.");

                auto group = std::make_shared<ConditionGroup>();
                QStringList negate;
                loadMember(negate, node->at("negate").get());

                if (const auto windowClassNode = node->at("window_class")) {
                    const auto value = Value(windowClassNode->as<QString>());
                    auto classGroup = std::make_shared<ConditionGroup>(ConditionGroupMode::Any);
                    classGroup->add(std::make_shared<VariableCondition>("window_class", value, ComparisonOperator::Regex));
                    classGroup->add(std::make_shared<VariableCondition>("window_name", value, ComparisonOperator::Regex));
                    classGroup->setNegate(negate.contains("window_class"));
                    group->add(classGroup);
                }
                if (const auto &windowStateNode = node->at("window_state")) {
                    QStringList value;
                    loadMember(value, windowStateNode.get());

                    const auto trueValue = Value<bool>(true);
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
                condition = group;
                return;
            }
        }

        // Not in any group
        if (node->raw()->IsSequence()) {
            auto group = std::make_shared<ConditionGroup>(isLegacy(node->raw()[0]) ? ConditionGroupMode::Any : ConditionGroupMode::All);
            for (const auto &child : node->sequenceChildren()) {
                group->add(child->as<std::shared_ptr<Condition>>());
            }
            condition = group;
            return;
        }

        if (node->raw()->IsScalar()) {
            // Hack to load negated conditions without forcing users to quote the entire thing
            auto conditionNode = node->clone();
            const auto tag = node->raw()->Tag();
            if (tag != "!" && tag.starts_with('!')) {
                *conditionNode.raw() = QString("%1 %2").arg(QString::fromStdString(tag), node->as<QString>()).trimmed().toStdString();
            }

            const auto raw = conditionNode.as<QString>();
            if (raw.startsWith("$") || raw.startsWith("!$")) {
                condition = parseVariableCondition(&conditionNode);
            }
        }
    }
};

template<>
struct NodeParser<std::vector<InputAction::Item>>
{
    static void parse(const Node *node, std::vector<InputAction::Item> &result)
    {
        for (const auto &device : node->sequenceChildren()) {
            if (const auto keyboardNode = device->at("keyboard")) {
                for (const auto &actionNode : keyboardNode->sequenceChildren()) {
                    if (actionNode->raw()->IsMap()) {
                        if (const auto textNode = actionNode->at("text")) {
                            result.push_back({
                                .keyboardText = textNode->as<Value<QString>>(),
                            });
                        }
                    } else {
                        const auto actionRaw = actionNode->as<QString>().toUpper();
                        if (actionRaw.startsWith("+") || actionRaw.startsWith("-")) {
                            const auto key = parseKeyboardKey(actionNode.get(), actionRaw.mid(1));

                            if (actionRaw[0] == '+') {
                                result.push_back({
                                    .keyboardPress = key,
                                });
                            } else {
                                result.push_back({
                                    .keyboardRelease = key,
                                });
                            }
                        } else {
                            std::vector<uint32_t> keys;
                            for (const auto &keyRaw : actionRaw.split("+")) {
                                keys.push_back(parseKeyboardKey(actionNode.get(), keyRaw));
                            }

                            for (const auto key : keys) {
                                result.push_back({
                                    .keyboardPress = key,
                                });
                            }
                            std::reverse(keys.begin(), keys.end());
                            for (const auto key : keys) {
                                result.push_back({
                                    .keyboardRelease = key,
                                });
                            }
                        }
                    }
                }
            } else if (const auto mouseNode = device->at("mouse")) {
                for (const auto &actionRaw : mouseNode->as<QStringList>()) {
                    const auto split = actionRaw.split(' ');
                    const auto action = split[0].toUpper();
                    const auto arguments = split.mid(1);

                    if (action.startsWith("+") || action.startsWith("-")) {
                        const auto button = parseMouseButton(mouseNode.get(), action.mid(1));

                        if (action[0] == '+') {
                            result.push_back({
                                .mousePress = button,
                            });
                        } else {
                            result.push_back({
                                .mouseRelease = button,
                            });
                        }
                    } else if (action.startsWith("MOVE_BY_DELTA")) {
                        result.push_back({
                            .mouseMoveRelativeByDelta = true,
                        });
                    } else if (action.startsWith("MOVE_BY")) {
                        result.push_back({
                            .mouseMoveRelative = parseMouseInputActionPoint(mouseNode.get(), arguments),
                        });
                    } else if (action.startsWith("MOVE_TO")) {
                        result.push_back({
                            .mouseMoveAbsolute = parseMouseInputActionPoint(mouseNode.get(), arguments),
                        });
                    } else if (action.startsWith("WHEEL")) {
                        result.push_back({
                            .mouseAxis = parseMouseInputActionPoint(mouseNode.get(), arguments),
                        });
                    } else {
                        std::vector<uint32_t> buttons;
                        for (const auto &buttonRaw : action.split("+")) {
                            buttons.push_back(parseMouseButton(mouseNode.get(), buttonRaw));
                        }

                        for (const auto button : buttons) {
                            result.push_back({
                                .mousePress = button,
                            });
                        }
                        std::reverse(buttons.begin(), buttons.end());
                        for (const auto button : buttons) {
                            result.push_back({
                                .mouseRelease = button,
                            });
                        }
                    }
                }
            }
        }
    }
};

template<>
void NodeParser<InputDeviceProperties>::parse(const Node *node, InputDeviceProperties &result)
{
    loadSetter(result, &InputDeviceProperties::setMultiTouch, node->at("__multiTouch").get());
    loadSetter(result, &InputDeviceProperties::setButtonPad, node->at("buttonpad").get());
    loadSetter(result, &InputDeviceProperties::setGrab, node->at("grab").get());
    loadSetter(result, &InputDeviceProperties::setHandleLibevdevEvents, node->at("handle_libevdev_events").get());
    loadSetter(result, &InputDeviceProperties::setIgnore, node->at("ignore").get());

    if (const auto pressureRangesNode = node->mapAt("pressure_ranges")) {
        loadSetter(result, &InputDeviceProperties::setFingerPressure, pressureRangesNode->at("finger").get());
        loadSetter(result, &InputDeviceProperties::setThumbPressure, pressureRangesNode->at("thumb").get());
        loadSetter(result, &InputDeviceProperties::setPalmPressure, pressureRangesNode->at("palm").get());
    }
}

template<>
void NodeParser<std::vector<InputDeviceRule>>::parse(const Node *node, std::vector<InputDeviceRule> &result)
{
    if (const auto rulesNode = node->at("device_rules")) {
        for (const auto &ruleNode : rulesNode->sequenceChildren()) {
            InputDeviceRule rule;
            loadSetter(rule, &InputDeviceRule::setCondition, ruleNode->at("conditions").get());
            loadSetter(rule, &InputDeviceRule::setProperties, ruleNode.get());
            result.push_back(std::move(rule));
        }
    }

    // Legacy
    if (const auto touchpadNode = node->mapAt("touchpad")) {
    touchpadNode->disableMapAccessCheck("gestures");

        if (const auto devicesNode = touchpadNode->mapAt("devices")) {
            g_config->addIssue(devicesNode.get(), ConfigIssueSeverity::Deprecation, "This method of defining device properties is deprecated. Use device_rules instead.");
            devicesNode->disableMapAccessCheck();

            for (auto [key, value] : devicesNode->mapChildren()) {
                InputDeviceRule rule;
                rule.setCondition(std::make_shared<VariableCondition>("name", Value(key->as<QString>()), ComparisonOperator::EqualTo));
                loadSetter(rule, &InputDeviceRule::setProperties, value.get());
                result.push_back(std::move(rule));

                key->disableMapAccessCheck();
            }
        }
    }
}

template<>
void NodeParser<KeyboardKey>::parse(const Node *node, KeyboardKey &result)
{
    const auto raw = node->as<QString>();
    if (const auto key = KeyboardKey::fromString(raw.toUpper())) {
        result = key.value();
        return;
    }

    throw ConfigParserException(node, QString("Invalid keyboard key '%1'").arg(raw));
}

template<>
void NodeParser<KeyboardShortcut>::parse(const Node *node, KeyboardShortcut &result)
{
    loadMember(result.keys, std::move(node));
}

template<typename T>
struct NodeParser<Range<T>>
{
    static void parse(const Node *node, Range<T> &result)
    {
        const auto raw = node->as<QString>().replace(" ", "");
        if (!raw.contains("-")) {
            result = Range<T>(node->as<T>(), {});
            return;
        }

        const auto split = raw.split("-", Qt::SkipEmptyParts);
        if (split.count() != 2) {
            throw ConfigParserException(node, "Invalid range, correct format is 'min-max'.");
        }

        Node minNode(split[0], node);
        Node maxNode(split[1], node);

        result = Range<T>(minNode.as<T>(), maxNode.as<T>());
    }
};

template<>
void NodeParser<std::unique_ptr<Trigger>>::parse(const Node *node, std::unique_ptr<Trigger> &result)
{
    const auto typeNode = node->at("type", true);
    auto type = typeNode->as<QString>();

    if (type == "circle") {
        result = std::make_unique<DirectionalMotionTrigger>(TriggerType::Circle, static_cast<TriggerDirection>(node->at("direction", true)->as<RotateDirection>()));
    } else if (type == "click") {
        result = std::make_unique<Trigger>(TriggerType::Click);
    } else if (type == "hold" || type == "press") {
        auto pressTrigger = new PressTrigger;
        loadSetter(pressTrigger, &PressTrigger::setInstant, node->at("instant").get());
        result.reset(pressTrigger);
    } else if (type == "hover") {
        result = std::make_unique<HoverTrigger>();
    } else if (type == "pinch") {
        result = std::make_unique<DirectionalMotionTrigger>(TriggerType::Pinch, static_cast<TriggerDirection>(node->at("direction", true)->as<PinchDirection>()));
    } else if (type == "rotate") {
        result = std::make_unique<DirectionalMotionTrigger>(TriggerType::Rotate, static_cast<TriggerDirection>(node->at("direction", true)->as<RotateDirection>()));
    } else if (type == "shortcut") {
        result = std::make_unique<KeyboardShortcutTrigger>(node->at("shortcut", true)->as<KeyboardShortcut>());
    } else if (type == "stroke") {
        result = std::make_unique<StrokeTrigger>(node->at("strokes", true)->asSequence().as<std::vector<Stroke>>());
    } else if (type == "swipe") {
        result = std::make_unique<DirectionalMotionTrigger>(TriggerType::Swipe, static_cast<TriggerDirection>(node->at("direction", true)->as<SwipeDirection>()));
    } else if (type == "tap") {
        result = std::make_unique<Trigger>(TriggerType::Tap);
    } else if (type == "wheel") {
        result = std::make_unique<WheelTrigger>(static_cast<TriggerDirection>(node->at("direction", true)->as<SwipeDirection>()));
    } else {
        throw ConfigParserException(typeNode.get(), "Invalid trigger type.");
    }

    loadSetter(result, &Trigger::setBlockEvents, node->at("block_events").get());
    loadSetter(result, &Trigger::setClearModifiers, node->at("clear_modifiers").get());
    loadSetter(result, &Trigger::setEndCondition, node->at("end_conditions").get());
    loadSetter(result, &Trigger::setId, node->at("id").get());
    loadSetter(result, &Trigger::setMouseButtons, node->at("mouse_buttons").get());
    loadSetter(result, &Trigger::setMouseButtonsExactOrder, node->at("mouse_buttons_exact_order").get());
    loadSetter(result, &Trigger::setResumeTimeout, node->at("resume_timeout").get());
    loadSetter(result, &Trigger::setSetLastTrigger, node->at("set_last_trigger").get());
    loadSetter(result, &Trigger::setThreshold, node->at("threshold").get());
    if (auto *motionTrigger = dynamic_cast<MotionTrigger *>(result.get())) {
        loadSetter(motionTrigger, &MotionTrigger::setLockPointer, node->at("lock_pointer").get());
        loadSetter(motionTrigger, &MotionTrigger::setSpeed, node->at("speed").get());
    }

    auto conditionGroup = std::make_shared<ConditionGroup>();
    if (const auto fingersNode = node->at("fingers")) {
        auto range = fingersNode->as<Range<qreal>>();
        if (!range.max()) {
            conditionGroup->add(std::make_shared<VariableCondition>(BuiltinVariables::Fingers,
                                                                    Value<qreal>(range.min().value()),
                                                                    ComparisonOperator::EqualTo));
        } else {
            conditionGroup->add(std::make_shared<VariableCondition>(BuiltinVariables::Fingers,
                                                                    std::vector<Value<std::any>>{
                                                                        Value<qreal>(range.min().value()),
                                                                        Value<qreal>(range.max().value())},
                                                                    ComparisonOperator::Between));
        }
    }
    if (const auto modifiersNode = node->at("keyboard_modifiers")) {
        std::optional<Qt::KeyboardModifiers> modifiers;
        if (modifiersNode->raw()->IsSequence()) {
            modifiers = modifiersNode->as<Qt::KeyboardModifiers>();
        } else {
            const auto modifierMatchingMode = modifiersNode->as<QString>();
            if (modifierMatchingMode == "none") {
                modifiers = Qt::KeyboardModifier::NoModifier;
            } else if (modifierMatchingMode != "any") {
                throw ConfigParserException(node, "Invalid keyboard modifier");
            }
        }

        if (modifiers) {
            conditionGroup->add(std::make_shared<VariableCondition>(BuiltinVariables::KeyboardModifiers,
                                                                    Value<Qt::KeyboardModifiers>(modifiers.value()),
                                                                    ComparisonOperator::EqualTo));
        }
    }
    if (const auto conditionsNode = node->at("conditions")) {
        conditionGroup->add(conditionsNode->as<std::shared_ptr<Condition>>());
    }
    result->setActivationCondition(conditionGroup);

    bool accelerated{};
    loadMember(accelerated, node->at("accelerated").get());

    if (const auto actionsNode = node->at("actions")) {
        for (const auto &actionNode : actionsNode->sequenceChildren()) {
            auto action = actionNode->as<std::unique_ptr<TriggerAction>>();
            if (dynamic_cast<StrokeTrigger *>(result.get()) && action->on() != On::End) {
                throw ConfigParserException(actionNode.get(), "Stroke triggers only support 'on: end' actions.");
            }

            action->setAccelerated(accelerated);
            result->addAction(std::move(action));
        }
    } else {
        g_config->addIssue(node, ConfigIssueSeverity::Warning, "Trigger has no 'actions' property.");
    }
}

// Trigger list, handles triggers groups as well
template<>
void NodeParser<std::vector<std::unique_ptr<Trigger>>>::parse(const Node *node, std::vector<std::unique_ptr<Trigger>> &result)
{
    for (auto triggerNode : node->sequenceChildren()) {
        if (auto subTriggersNode = triggerNode->at("gestures")) {
            triggerNode->disableMapAccessCheck();
            subTriggersNode->disableMapAccessCheck();

            // Trigger group
            for (auto subTriggerNode : subTriggersNode->sequenceChildren()) {
                subTriggerNode->disableMapAccessCheck();

                // A copy of subTriggerNode is not used intentionally to preserve the mark. This effectively makes the node single-use only.

                auto clonedNode = *subTriggerNode->raw();

                std::shared_ptr<Condition> groupCondition;
                for (auto [key, value] : triggerNode->mapChildren()) {
                    const auto name = key->as<QString>();

                    if (name == "conditions") {
                        groupCondition = triggerNode->at("conditions")->as<std::shared_ptr<Condition>>();
                    } else if (name != "gestures") {
                        clonedNode[name.toStdString()] = *value->raw();
                    }
                }

                Node processedNode(clonedNode);
                processedNode.disableMapAccessCheck();

                for (auto &trigger : processedNode.asSequence().as<std::vector<std::unique_ptr<Trigger>>>()) {
                    if (groupCondition) {
                        auto conditionGroup = std::make_shared<ConditionGroup>();
                        if (const auto triggerCondition = trigger->activationCondition()) {
                            conditionGroup->add(triggerCondition);
                        }
                        conditionGroup->add(groupCondition);
                        trigger->setActivationCondition(conditionGroup);
                    }

                    result.push_back(std::move(trigger));
                }
            }
            continue;
        }

        result.push_back(triggerNode->as<std::unique_ptr<Trigger>>());
    }
}

template<>
void NodeParser<std::unique_ptr<TriggerAction>>::parse(const Node *node, std::unique_ptr<TriggerAction> &result)
{
    result = std::make_unique<TriggerAction>(node->as<std::shared_ptr<Action>>());

    loadSetter(result, &TriggerAction::setOn, node->at("on").get());

    if (const auto intervalNode = node->at("interval")) {
        if (result->on() != On::Update) {
            throw ConfigParserException(intervalNode.get(), "Intervals can only be set on 'on: update' actions.");
        }
        loadSetter(result, &TriggerAction::setInterval, intervalNode.get());
    }

    if (const auto thresholdNode = node->at("threshold")) {
        if (result->on() == On::Begin) {
            throw ConfigParserException(thresholdNode.get(), "Thresholds cannot be set on 'on: begin' actions.");
        }
        loadSetter(result, &TriggerAction::setThreshold, thresholdNode.get());
    }
}

template<>
void NodeParser<Stroke>::parse(const Node *node, Stroke &result)
{
    const auto bytes = QByteArray::fromBase64(node->as<QString>().toUtf8());
    if (bytes.size() % 4 != 0) {
        throw ConfigParserException(node, "Invalid stroke.");
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

    result = Stroke(points);
}

template<typename T>
struct NodeParser<Value<T>>
{
    static void parse(const Node *node, Value<T> &result)
    {
        if (node->raw()->IsMap()) {
            if (const auto commandNode = node->at("command")) {
                result = Value<T>::command(commandNode->as<Value<QString>>());
            }
        } else {
            const auto raw = node->as<QString>();
            const auto variableName = raw.mid(1); // remove $
            if (g_variableManager->hasVariable(variableName)) {
                result = Value<T>::variable(variableName);
            } else {
                result = Value<T>(node->as<T>());
            }
        }
    }
};

inline void parseTriggerHandler(const Node *node, TriggerHandler *handler)
{
    const auto triggersNode = node->at("gestures");
    if (!triggersNode) {
        throw ConfigParserException(node, "No gestures specified");
    }
    for (auto &trigger : triggersNode->as<std::vector<std::unique_ptr<Trigger>>>()) {
        handler->addTrigger(std::move(trigger));
    }
    loadSetter(handler, &TriggerHandler::setTimedTriggerUpdateDelta, node->at("__time_delta").get());
}

inline void parseMotionTriggerHandler(const Node *node, MotionTriggerHandler *handler)
{
    parseTriggerHandler(node, handler);

    if (const auto speedNode = node->mapAt("speed")) {
        loadSetter(handler, &MotionTriggerHandler::setInputEventsToSample, speedNode->at("events").get());
        if (auto thresholdNode = speedNode->at("swipe_threshold")) {
            handler->setSpeedThreshold(TriggerType::Swipe, thresholdNode->as<qreal>());
        }
    }
}

inline void parseMultiTouchMotionTriggerHandler(const Node *node, MultiTouchMotionTriggerHandler *handler)
{
    parseMotionTriggerHandler(node, handler);

    if (const auto speedNode = node->mapAt("speed")) {
        if (const auto thresholdNode = speedNode->at("pinch_in_threshold")) {
            handler->setSpeedThreshold(TriggerType::Pinch, thresholdNode->as<qreal>(), static_cast<TriggerDirection>(PinchDirection::In));
        }
        if (const auto thresholdNode = speedNode->at("pinch_out_threshold")) {
            handler->setSpeedThreshold(TriggerType::Pinch, thresholdNode->as<qreal>(), static_cast<TriggerDirection>(PinchDirection::Out));
        }
        if (const auto thresholdNode = speedNode->at("rotate_threshold")) {
            handler->setSpeedThreshold(TriggerType::Rotate, thresholdNode->as<qreal>());
        }
    }
}

std::unique_ptr<TouchpadTriggerHandler> parseTouchpadTriggerHandler(const Node *node, InputDevice *device)
{
    auto handler = std::make_unique<TouchpadTriggerHandler>(device);
    parseMultiTouchMotionTriggerHandler(node, handler.get());
    loadSetter(handler, &TouchpadTriggerHandler::setClickTimeout, node->at("click_timeout").get());
    loadSetter(static_cast<MotionTriggerHandler *>(handler.get()), &MotionTriggerHandler::setSwipeDeltaMultiplier, node->at("delta_multiplier").get());
    return handler;
}

std::unique_ptr<TouchscreenTriggerHandler> parseTouchscreenTriggerHandler(const Node *node, InputDevice *device)
{
    auto handler = std::make_unique<TouchscreenTriggerHandler>(device);
    parseMultiTouchMotionTriggerHandler(node, handler.get());
    return handler;
}

template<>
void NodeParser<std::unique_ptr<KeyboardTriggerHandler>>::parse(const Node *node, std::unique_ptr<KeyboardTriggerHandler> &result)
{
    result = std::make_unique<KeyboardTriggerHandler>();
    parseTriggerHandler(node, result.get());
}

template<>
void NodeParser<std::unique_ptr<MouseTriggerHandler>>::parse(const Node *node, std::unique_ptr<MouseTriggerHandler> &result)
{
    auto *mouseTriggerHandler = new MouseTriggerHandler;
    result.reset(mouseTriggerHandler);
    parseMotionTriggerHandler(node, result.get());

    loadSetter(mouseTriggerHandler, &MouseTriggerHandler::setMotionTimeout, node->at("motion_timeout").get());
    loadSetter(mouseTriggerHandler, &MouseTriggerHandler::setPressTimeout, node->at("press_timeout").get());
    loadSetter(mouseTriggerHandler, &MouseTriggerHandler::setUnblockButtonsOnTimeout, node->at("unblock_buttons_on_timeout").get());
}

template<>
void NodeParser<std::unique_ptr<PointerTriggerHandler>>::parse(const Node *node, std::unique_ptr<PointerTriggerHandler> &result)
{
    result = std::make_unique<PointerTriggerHandler>();
    parseTriggerHandler(node, result.get());
}

}
