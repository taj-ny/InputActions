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

#include "core.h"
#include "containers.h"
#include "flags.h"
#include "globals.h"
#include "separated-string.h"
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
#include <libinputactions/conditions/VariableCondition.h>
#include <libinputactions/config/ConfigIssue.h>
#include <libinputactions/config/ConfigIssueManager.h>
#include <libinputactions/config/Node.h>
#include <libinputactions/handlers/KeyboardTriggerHandler.h>
#include <libinputactions/handlers/MouseTriggerHandler.h>
#include <libinputactions/handlers/PointerTriggerHandler.h>
#include <libinputactions/handlers/TouchpadTriggerHandler.h>
#include <libinputactions/handlers/TouchscreenTriggerHandler.h>
#include <libinputactions/helpers/QString.h>
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

namespace InputActions
{

static Value<std::any> parseAny(const Node *node, const std::type_index &type)
{
    if (type == typeid(bool)) {
        return node->as<Value<bool>>();
    } else if (type == typeid(CursorShape)) {
        return node->as<Value<CursorShape>>();
    } else if (type == typeid(Qt::KeyboardModifiers)) {
        return Value<Qt::KeyboardModifiers>(node->as<Qt::KeyboardModifiers>(true));
    } else if (type == typeid(InputDeviceTypes)) {
        return Value<InputDeviceTypes>(node->as<InputDeviceTypes>(true));
    } else if (type == typeid(qreal)) {
        return node->as<Value<qreal>>();
    } else if (type == typeid(QPointF)) {
        return node->as<Value<QPointF>>();
    } else if (type == typeid(QString)) {
        return node->as<Value<QString>>();
    }
    throw InvalidValueConfigException(node, "Unexpected type.");
}

static std::shared_ptr<Condition> parseVariableCondition(const Node *node, const VariableManager *variableManager)
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
    const auto *variable = variableManager->getVariable(variableName);
    if (!variable) {
        // Variable type must be known in order to parse the right side of the condition
        throw InvalidVariableConfigException(node, variableName);
    }

    ComparisonOperator comparisonOperator;
    std::vector<Value<std::any>> right;
    if (firstSpace == -1 && variable->type() == typeid(bool)) { // bool variable condition without operator
        comparisonOperator = ComparisonOperator::EqualTo;
        right.push_back(Value<bool>(true));
    } else {
        const auto operatorNode = node->substringNodeQuoted(raw.mid(firstSpace + 1, secondSpace - firstSpace - 1));
        comparisonOperator = operatorNode->as<ComparisonOperator>();

        if (secondSpace == -1) {
            throw InvalidValueConfigException(node, "Missing value after operator.");
        }

        const auto rightRaw = raw.mid(secondSpace + 1);
        auto rightNode = node->substringNode(rightRaw);

        // Error checking
        if (comparisonOperator == ComparisonOperator::Regex) {
            rightNode->as<QRegularExpression>();
        }

        if (!isTypeFlags(variable->type()) && rightNode->isSequence()) {
            for (const auto *item : rightNode->sequenceItems()) {
                right.push_back(parseAny(item, variable->type()));
            }
        } else if (comparisonOperator == ComparisonOperator::Between) {
            const auto values = parseSeparatedString2<std::shared_ptr<Node>>(rightNode.get(), ';');
            right.push_back(parseAny(values.first.get(), variable->type()));
            right.push_back(parseAny(values.second.get(), variable->type()));
        } else {
            right.push_back(parseAny(rightNode.get(), variable->type()));
        }
    }

    auto condition = std::make_shared<VariableCondition>(variableName, right, comparisonOperator);
    condition->setNegate(negate);
    return condition;
}

template<>
void NodeParser<std::unique_ptr<Action>>::parse(const Node *node, std::unique_ptr<Action> &result)
{
    if (const auto *commandNode = node->at("command")) {
        auto action = std::make_unique<CommandAction>(commandNode->as<Value<QString>>());
        loadSetter(action, &CommandAction::setWait, node->at("wait"));
        result = std::move(action);
    } else if (const auto *inputNode = node->at("input")) {
        auto action = std::make_unique<InputAction>(inputNode->as<std::vector<InputActionItem>>());
        loadSetter(action, &InputAction::setDelay, node->at("delay"));
        result = std::move(action);
    } else if (const auto *plasmaShortcutNode = node->at("plasma_shortcut")) {
        const auto shortcut = parseSeparatedString2<QString>(plasmaShortcutNode, ',');
        result = std::make_unique<PlasmaGlobalShortcutAction>(shortcut.first, shortcut.second);
    } else if (const auto *sleepActionNode = node->at("sleep")) {
        result = std::make_unique<SleepAction>(sleepActionNode->as<std::chrono::milliseconds>());
    } else if (const auto *oneNode = node->at("one")) {
        result = std::make_unique<ActionGroup>(oneNode->as<std::vector<std::unique_ptr<Action>>>(), ActionGroup::ExecutionMode::First);
    } else {
        throw InvalidValueConfigException(node, "Action is missing a required property that determines its type.");
    }

    loadSetter(result, &Action::setCondition, node->at("conditions"));
    loadSetter(result, &Action::setExecutionLimit, node->at("limit"));
    loadSetter(result, &Action::setId, node->at("id"));
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

std::shared_ptr<Condition> parseCondition(const Node *node, const VariableManager *variableManager)
{
    if (!variableManager) {
        variableManager = g_variableManager.get();
    }

    static const auto isLegacy = [](const Node *node) {
        return node->isMap() && (node->at("negate") || node->at("window_class") || node->at("window_state"));
    };

    if (node->isMap()) {
        std::optional<ConditionGroupMode> groupMode;
        const Node *groupChildren;
        if (const auto *allNode = node->at("all")) {
            groupMode = ConditionGroupMode::All;
            groupChildren = allNode;
        } else if (const auto *anyNode = node->at("any")) {
            groupMode = ConditionGroupMode::Any;
            groupChildren = anyNode;
        } else if (const auto *noneNode = node->at("none")) {
            groupMode = ConditionGroupMode::None;
            groupChildren = noneNode;
        }
        if (groupMode) {
            auto group = std::make_shared<ConditionGroup>(*groupMode);
            for (const auto &item : groupChildren->sequenceItems()) {
                group->append(parseCondition(item, variableManager));
            }
            return group;
        }

        if (isLegacy(node)) {
            g_configIssueManager->addIssue(DeprecatedFeatureConfigIssue(node, DeprecatedFeature::LegacyConditions));

            auto group = std::make_shared<ConditionGroup>();
            QStringList negate;
            loadMember(negate, node->at("negate"));

            if (const auto *windowClassNode = node->at("window_class")) {
                const auto value = Value(windowClassNode->as<QString>());
                auto classGroup = std::make_shared<ConditionGroup>(ConditionGroupMode::Any);
                classGroup->append(std::make_shared<VariableCondition>("window_class", value, ComparisonOperator::Regex));
                classGroup->append(std::make_shared<VariableCondition>("window_name", value, ComparisonOperator::Regex));
                classGroup->setNegate(negate.contains("window_class"));
                group->append(classGroup);
            }
            if (const auto *windowStateNode = node->at("window_state")) {
                QStringList value;
                loadMember(value, windowStateNode);

                const auto trueValue = Value<bool>(true);
                auto classGroup = std::make_shared<ConditionGroup>(ConditionGroupMode::Any);
                if (value.contains("fullscreen")) {
                    classGroup->append(std::make_shared<VariableCondition>("window_fullscreen", trueValue, ComparisonOperator::EqualTo));
                }
                if (value.contains("maximized")) {
                    classGroup->append(std::make_shared<VariableCondition>("window_maximized", trueValue, ComparisonOperator::EqualTo));
                }
                classGroup->setNegate(negate.contains("window_state"));
                group->append(classGroup);
            }

            const auto &groupChildren = group->conditions();
            if (groupChildren.size() == 1) {
                return groupChildren[0];
            }
            return group;
        }
    }

    // Not in any group
    if (node->isSequence()) {
        static const auto legacyPred = [](const auto &node) {
            return isLegacy(node);
        };

        const auto items = node->sequenceItems();
        const auto hasLegacyCondition = std::ranges::any_of(items, legacyPred);
        if (hasLegacyCondition && !std::ranges::all_of(items, legacyPred)) {
            throw InvalidValueContextConfigException(node, "Mixing legacy and normal conditions is not allowed.");
        }

        auto group = std::make_shared<ConditionGroup>(hasLegacyCondition ? ConditionGroupMode::Any : ConditionGroupMode::All);
        for (const auto &item : items) {
            group->append(parseCondition(item, variableManager));
        }
        return group;
    }

    if (node->isScalar()) {
        // Hack to load negated conditions without forcing users to quote the entire thing
        auto conditionNode = node->shared_from_this();

        const auto tag = conditionNode->tag();
        if (tag != "!" && tag.startsWith('!')) {
            conditionNode = node->substringNodeQuoted(QString("%1 %2").arg(tag, node->as<QString>()).trimmed());
        }

        const auto raw = conditionNode->as<QString>();
        if (raw.startsWith("$") || raw.startsWith("!$")) {
            return parseVariableCondition(conditionNode.get(), variableManager);
        }
    }

    throw InvalidValueConfigException(node, "Invalid condition.");
}

template<>
void NodeParser<std::shared_ptr<Condition>>::parse(const Node *node, std::shared_ptr<Condition> &condition)
{
    condition = parseCondition(node);
}

template<>
void NodeParser<std::vector<InputActionItem>>::parse(const Node *node, std::vector<InputActionItem> &result)
{
    for (const auto *deviceNode : node->sequenceItems()) {
        if (const auto *keyboardNode = deviceNode->at("keyboard")) {
            for (const auto *actionNode : keyboardNode->sequenceItems()) {
                if (actionNode->isMap()) {
                    if (const auto *textNode = actionNode->at("text")) {
                        result.push_back({
                            .keyboardText = textNode->as<Value<QString>>(),
                        });
                    }
                } else {
                    const auto actionRaw = actionNode->as<QString>().toLower();
                    if (actionRaw.startsWith("+") || actionRaw.startsWith("-")) {
                        const auto key = actionNode->substringNode(actionRaw.mid(1))->as<KeyboardKey>();

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
                        if (actionRaw.contains("++")) {
                            throw InvalidValueConfigException(actionNode,
                                                              "Syntax error: found at least two '+' characters next to each other with no key in between.");
                        }
                        if (actionRaw.endsWith("+")) {
                            throw InvalidValueConfigException(actionNode, "Syntax error: found trailing '+' character with no key after.");
                        }

                        std::vector<KeyboardKey> keys;
                        for (const auto &keyRaw : actionRaw.split("+")) {
                            keys.push_back(actionNode->substringNode(keyRaw)->as<KeyboardKey>());
                        }

                        for (const auto key : keys) {
                            result.push_back({
                                .keyboardPress = key,
                            });
                        }
                        std::ranges::reverse(keys);
                        for (const auto key : keys) {
                            result.push_back({
                                .keyboardRelease = key,
                            });
                        }
                    }
                }
            }
        } else if (const auto *mouseNode = deviceNode->at("mouse")) {
            for (const auto *actionNode : mouseNode->sequenceItems()) {
                const auto actionRaw = actionNode->as<QString>();
                if (actionRaw.contains("  ")) {
                    throw InvalidValueConfigException(actionNode, "Syntax error: found at least two space characters next to each other.");
                }

                const auto split = actionRaw.split(' ');
                const auto action = split[0].toUpper();
                const auto arguments = split.mid(1).join(",");

                if (action.startsWith("+") || action.startsWith("-")) {
                    const auto button = actionNode->substringNode(action.mid(1))->as<MouseButton>();

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
                    qreal multiplier = 1;
                    if (!arguments.isEmpty()) {
                        multiplier = actionNode->substringNode(arguments)->as<qreal>();
                    }

                    result.push_back({
                        .mouseMoveRelativeByDelta = multiplier,
                    });
                } else if (action.startsWith("MOVE_BY")) {
                    result.push_back({
                        .mouseMoveRelative = actionNode->substringNodeQuoted(arguments)->as<QPointF>(),
                    });
                } else if (action.startsWith("MOVE_TO")) {
                    result.push_back({
                        .mouseMoveAbsolute = actionNode->substringNodeQuoted(arguments)->as<QPointF>(),
                    });
                } else if (action.startsWith("WHEEL")) {
                    result.push_back({
                        .mouseAxis = actionNode->substringNodeQuoted(arguments)->as<QPointF>(),
                    });
                } else {
                    if (actionRaw.contains("++")) {
                        throw InvalidValueConfigException(actionNode,
                                                          "Syntax error: found at least two '+' characters next to each other with no button in between.");
                    }
                    if (actionRaw.endsWith("+")) {
                        throw InvalidValueConfigException(actionNode, "Syntax error: found trailing '+' character with no button after.");
                    }

                    std::vector<MouseButton> buttons;
                    for (const auto &buttonRaw : actionRaw.split("+")) {
                        buttons.push_back(actionNode->substringNode(buttonRaw)->as<MouseButton>());
                    }

                    for (const auto button : buttons) {
                        result.push_back({
                            .mousePress = button,
                        });
                    }
                    std::ranges::reverse(buttons);
                    for (const auto button : buttons) {
                        result.push_back({
                            .mouseRelease = button,
                        });
                    }
                }
            }
        } else {
            throw InvalidValueConfigException(deviceNode, "Invalid device type.");
        }
    }
}

template<>
void NodeParser<InputDeviceProperties>::parse(const Node *node, InputDeviceProperties &result)
{
    loadSetter(result, &InputDeviceProperties::setMultiTouch, node->at("__multiTouch"));
    loadSetter(result, &InputDeviceProperties::setGrab, node->at("grab"));
    loadSetter(result, &InputDeviceProperties::setHandleLibevdevEvents, node->at("handle_libevdev_events"));
    loadSetter(result, &InputDeviceProperties::setIgnore, node->at("ignore"));
    loadSetter(result, &InputDeviceProperties::setMouseMotionTimeout, node->at("motion_timeout"));
    loadSetter(result, &InputDeviceProperties::setMousePressTimeout, node->at("press_timeout"));
    loadSetter(result, &InputDeviceProperties::setMouseUnblockButtonsOnTimeout, node->at("unblock_buttons_on_timeout"));
    loadSetter(result, &InputDeviceProperties::setTouchpadButtonPad, node->at("buttonpad"));
    loadSetter(result, &InputDeviceProperties::setTouchpadClickTimeout, node->at("click_timeout"));

    if (const auto *pressureRangesNode = node->mapAt("pressure_ranges")) {
        loadSetter(result, &InputDeviceProperties::setFingerPressure, pressureRangesNode->at("finger"));
        loadSetter(result, &InputDeviceProperties::setThumbPressure, pressureRangesNode->at("thumb"));
        loadSetter(result, &InputDeviceProperties::setPalmPressure, pressureRangesNode->at("palm"));
    }
}

template<>
void NodeParser<std::vector<InputDeviceRule>>::parse(const Node *node, std::vector<InputDeviceRule> &result)
{
    if (const auto *rulesNode = node->at("device_rules")) {
        for (const auto *ruleNode : rulesNode->sequenceItems()) {
            InputDeviceRule rule;
            if (const auto *conditionsNode = ruleNode->at("conditions")) {
                rule.setCondition(parseCondition(conditionsNode, &g_inputBackend->deviceRulesVariableManager()));
            }
            loadSetter(rule, &InputDeviceRule::setProperties, ruleNode);
            result.push_back(std::move(rule));
        }
    }

    // Legacy
    if (const auto *mouseNode = node->mapAt("mouse")) {
        const auto *motionTimeoutNode = mouseNode->at("motion_timeout");
        if (motionTimeoutNode) {
            g_configIssueManager->addIssue(DeprecatedFeatureConfigIssue(motionTimeoutNode, DeprecatedFeature::TriggerHandlerSettings));
        }
        const auto *pressTimeoutNode = mouseNode->at("press_timeout");
        if (pressTimeoutNode) {
            g_configIssueManager->addIssue(DeprecatedFeatureConfigIssue(pressTimeoutNode, DeprecatedFeature::TriggerHandlerSettings));
        }
        const auto *unblockButtonsOnTimeout = mouseNode->at("unblock_buttons_on_timeout");
        if (unblockButtonsOnTimeout) {
            g_configIssueManager->addIssue(DeprecatedFeatureConfigIssue(unblockButtonsOnTimeout, DeprecatedFeature::TriggerHandlerSettings));
        }

        if (motionTimeoutNode || pressTimeoutNode || unblockButtonsOnTimeout) {
            InputDeviceRule rule;
            rule.setCondition(std::make_shared<VariableCondition>("mouse", InputActions::Value(true), ComparisonOperator::EqualTo));
            loadSetter(rule.properties(), &InputDeviceProperties::setMouseMotionTimeout, motionTimeoutNode);
            loadSetter(rule.properties(), &InputDeviceProperties::setMousePressTimeout, pressTimeoutNode);
            loadSetter(rule.properties(), &InputDeviceProperties::setMouseUnblockButtonsOnTimeout, unblockButtonsOnTimeout);
            result.push_back(std::move(rule));
        }
    }

    if (const auto *touchpadNode = node->mapAt("touchpad")) {
        if (const auto *clickTimeoutNode = touchpadNode->at("click_timeout")) {
            g_configIssueManager->addIssue(DeprecatedFeatureConfigIssue(clickTimeoutNode, DeprecatedFeature::TriggerHandlerSettings));

            InputDeviceRule rule;
            rule.setCondition(std::make_shared<VariableCondition>("touchpad", InputActions::Value(true), ComparisonOperator::EqualTo));
            loadSetter(rule.properties(), &InputDeviceProperties::setTouchpadClickTimeout, clickTimeoutNode);
            result.push_back(std::move(rule));
        }

        if (const auto *devicesNode = touchpadNode->mapAt("devices")) {
            g_configIssueManager->addIssue(DeprecatedFeatureConfigIssue(devicesNode, DeprecatedFeature::TouchpadDevicesNode));

            for (const auto &[key, value] : devicesNode->mapItems()) {
                value->markUsed();
                InputDeviceRule rule;
                rule.setCondition(std::make_shared<VariableCondition>("name", Value(key), ComparisonOperator::EqualTo));
                loadSetter(rule, &InputDeviceRule::setProperties, value);
                result.push_back(std::move(rule));
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

    throw InvalidValueConfigException(node, QString("Invalid keyboard key '%1'.").arg(raw));
}

template<>
void NodeParser<MouseButton>::parse(const Node *node, MouseButton &result)
{
    const auto raw = node->as<QString>();
    if (const auto button = MouseButton::fromString(raw.toUpper())) {
        result = button.value();
        return;
    }

    throw InvalidValueConfigException(node, QString("Invalid mouse button '%1'.").arg(raw));
}

template<>
void NodeParser<KeyboardShortcut>::parse(const Node *node, KeyboardShortcut &result)
{
    loadMember(result.keys, node);
}

template<typename T>
struct NodeParser<Range<T>>
{
    static void parse(const Node *node, Range<T> &result)
    {
        const auto raw = node->as<QString>();
        if (!raw.contains("-")) {
            result = Range<T>(node->as<T>(), {});
            return;
        }

        const auto values = parseSeparatedString2<T>(node, '-');
        result = Range<T>(values.first, values.second);
    }
};
template struct NodeParser<Range<qreal>>;

template<>
void NodeParser<std::unique_ptr<Trigger>>::parse(const Node *node, std::unique_ptr<Trigger> &result)
{
    const auto *typeNode = node->at("type", true);
    auto type = typeNode->as<QString>();

    if (type == "circle") {
        result = std::make_unique<DirectionalMotionTrigger>(TriggerType::Circle,
                                                            static_cast<TriggerDirection>(node->at("direction", true)->as<RotateDirection>()));
    } else if (type == "click") {
        result = std::make_unique<Trigger>(TriggerType::Click);
    } else if (type == "hold" || type == "press") {
        auto pressTrigger = std::make_unique<PressTrigger>();
        loadSetter(pressTrigger.get(), &PressTrigger::setInstant, node->at("instant"));
        result = std::move(pressTrigger);
    } else if (type == "hover") {
        result = std::make_unique<HoverTrigger>();
    } else if (type == "pinch") {
        result = std::make_unique<DirectionalMotionTrigger>(TriggerType::Pinch,
                                                            static_cast<TriggerDirection>(node->at("direction", true)->as<PinchDirection>()));
    } else if (type == "rotate") {
        result = std::make_unique<DirectionalMotionTrigger>(TriggerType::Rotate,
                                                            static_cast<TriggerDirection>(node->at("direction", true)->as<RotateDirection>()));
    } else if (type == "shortcut") {
        result = std::make_unique<KeyboardShortcutTrigger>(node->at("shortcut", true)->as<KeyboardShortcut>());
    } else if (type == "stroke") {
        result = std::make_unique<StrokeTrigger>(node->at("strokes", true)->as<std::vector<Stroke>>(true));
    } else if (type == "swipe") {
        result = std::make_unique<DirectionalMotionTrigger>(TriggerType::Swipe,
                                                            static_cast<TriggerDirection>(node->at("direction", true)->as<SwipeDirection>()));
    } else if (type == "tap") {
        result = std::make_unique<Trigger>(TriggerType::Tap);
    } else if (type == "wheel") {
        result = std::make_unique<WheelTrigger>(static_cast<TriggerDirection>(node->at("direction", true)->as<SwipeDirection>()));
    } else {
        throw InvalidValueConfigException(typeNode, QString("Invalid trigger type '%1'.").arg(type));
    }

    loadSetter(result, &Trigger::setBlockEvents, node->at("block_events"));
    loadSetter(result, &Trigger::setClearModifiers, node->at("clear_modifiers"));
    loadSetter(result, &Trigger::setEndCondition, node->at("end_conditions"));
    loadSetter(result, &Trigger::setId, node->at("id"));
    loadSetter(result, &Trigger::setMouseButtonsExactOrder, node->at("mouse_buttons_exact_order"));
    loadSetter(result, &Trigger::setResumeTimeout, node->at("resume_timeout"));
    loadSetter(result, &Trigger::setSetLastTrigger, node->at("set_last_trigger"));
    loadSetter(result, &Trigger::setThreshold, node->at("threshold"));
    if (const auto *mouseButtonsNode = node->at("mouse_buttons")) {
        mouseButtonsNode->as<std::set<MouseButton>>(); // Ensure no duplicates, Trigger::setMouseButtons accepts a vector
        loadSetter(result, &Trigger::setMouseButtons, mouseButtonsNode);
    }
    if (auto *motionTrigger = dynamic_cast<MotionTrigger *>(result.get())) {
        loadSetter(motionTrigger, &MotionTrigger::setLockPointer, node->at("lock_pointer"));
        loadSetter(motionTrigger, &MotionTrigger::setSpeed, node->at("speed"));
    }

    auto conditionGroup = std::make_shared<ConditionGroup>();
    if (const auto *fingersNode = node->at("fingers")) {
        auto range = fingersNode->as<Range<qreal>>();
        if (!range.max()) {
            conditionGroup->append(std::make_shared<VariableCondition>(BuiltinVariables::Fingers,
                                                                       Value<qreal>(range.min().value()),
                                                                       ComparisonOperator::EqualTo));
        } else {
            conditionGroup->append(std::make_shared<VariableCondition>(BuiltinVariables::Fingers,
                                                                       std::vector<Value<std::any>>{
                                                                           Value<qreal>(range.min().value()), Value<qreal>(range.max().value())},
                                                                       ComparisonOperator::Between));
        }
    }
    if (const auto *modifiersNode = node->at("keyboard_modifiers")) {
        g_configIssueManager->addIssue(DeprecatedFeatureConfigIssue(modifiersNode, DeprecatedFeature::TriggerKeyboardModifiers));

        std::optional<Qt::KeyboardModifiers> modifiers;
        if (modifiersNode->isSequence()) {
            modifiers = modifiersNode->as<Qt::KeyboardModifiers>();
        } else {
            const auto modifierMatchingMode = modifiersNode->as<QString>();
            if (modifierMatchingMode == "none") {
                modifiers = Qt::KeyboardModifier::NoModifier;
            } else if (modifierMatchingMode != "any") {
                throw InvalidValueConfigException(modifiersNode, "Invalid keyboard modifier.");
            }
        }

        if (modifiers) {
            conditionGroup->append(std::make_shared<VariableCondition>(BuiltinVariables::KeyboardModifiers,
                                                                       Value<Qt::KeyboardModifiers>(modifiers.value()),
                                                                       ComparisonOperator::EqualTo));
        }
    }
    if (const auto *conditionsNode = node->at("conditions")) {
        conditionGroup->append(conditionsNode->as<std::shared_ptr<Condition>>());
    }

    if (conditionGroup->conditions().size() == 1) {
        result->setActivationCondition(conditionGroup->conditions()[0]);
    } else if (conditionGroup->conditions().size()) {
        result->setActivationCondition(conditionGroup);
    }

    bool accelerated{};
    loadMember(accelerated, node->at("accelerated"));

    if (const auto *actionsNode = node->at("actions")) {
        for (const auto *actionNode : actionsNode->sequenceItems()) {
            auto action = actionNode->as<std::unique_ptr<TriggerAction>>();
            if (dynamic_cast<StrokeTrigger *>(result.get()) && action->on() != On::End && action->conflicting()) {
                throw InvalidValueContextConfigException(actionNode, "Stroke triggers only support 'on: end' conflicting actions.");
            }

            action->setAccelerated(accelerated);
            result->addAction(std::move(action));
        }
    }
}

// Trigger list, handles triggers groups as well
template<>
void NodeParser<std::vector<std::unique_ptr<Trigger>>>::parse(const Node *node, std::vector<std::unique_ptr<Trigger>> &result)
{
    for (const auto *triggerNode : node->sequenceItems()) {
        if (const auto *subTriggersNode = triggerNode->at("gestures")) {
            // Trigger group
            for (const auto *subTriggerNode : subTriggersNode->sequenceItems()) {
                const auto mergedNode = std::make_shared<Node>(*subTriggerNode);

                std::shared_ptr<Condition> groupCondition;
                for (const auto &[key, value] : triggerNode->mapItemsRawKeys()) {
                    const auto keyStr = key->as<QString>();
                    if (keyStr == "conditions") {
                        groupCondition = triggerNode->at("conditions")->as<std::shared_ptr<Condition>>();
                    } else if (keyStr != "gestures") {
                        mergedNode->addMapItem(key->shared_from_this(), value->shared_from_this());
                    }
                }
                for (auto &trigger : mergedNode->as<std::vector<std::unique_ptr<Trigger>>>(true)) {
                    if (groupCondition) {
                        if (const auto triggerCondition = trigger->activationCondition()) {
                            if (const auto triggerConditionGroup = std::dynamic_pointer_cast<ConditionGroup>(triggerCondition);
                                triggerConditionGroup && triggerConditionGroup->mode() == ConditionGroupMode::All) {
                                triggerConditionGroup->prepend(groupCondition);
                            } else {
                                auto conditionGroup = std::make_shared<ConditionGroup>();
                                conditionGroup->append(groupCondition);
                                conditionGroup->append(trigger->activationCondition());
                                trigger->setActivationCondition(conditionGroup);
                            }
                        } else {
                            trigger->setActivationCondition(groupCondition);
                        }
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
    result = std::make_unique<TriggerAction>(node->as<std::unique_ptr<Action>>());

    loadSetter(result, &TriggerAction::setConflicting, node->at("conflicting"));
    loadSetter(result, &TriggerAction::setOn, node->at("on"));

    if (const auto *intervalNode = node->at("interval")) {
        if (result->on() != On::Update && result->on() != On::Tick) {
            throw InvalidValueContextConfigException(intervalNode, "Intervals can only be set on update and tick actions.");
        }
        loadSetter(result, &TriggerAction::setInterval, intervalNode);
    }

    if (const auto *thresholdNode = node->at("threshold")) {
        if (result->on() == On::Begin) {
            throw InvalidValueContextConfigException(thresholdNode, "Thresholds cannot be set on begin actions.");
        }
        loadSetter(result, &TriggerAction::setThreshold, thresholdNode);
    }
}

template<>
void NodeParser<Stroke>::parse(const Node *node, Stroke &result)
{
    const auto bytes = QByteArray::fromBase64(node->as<QString>().toUtf8());
    if (bytes.size() % 4 != 0) {
        throw InvalidValueConfigException(node, "Invalid stroke.");
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
        if (node->isMap()) {
            if (const auto *commandNode = node->at("command")) {
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
    for (auto &trigger : node->at("gestures", true)->as<std::vector<std::unique_ptr<Trigger>>>()) {
        handler->addTrigger(std::move(trigger));
    }
    loadSetter(handler, &TriggerHandler::setTimedTriggerUpdateDelta, node->at("__time_delta"));
}

inline void parseMotionTriggerHandler(const Node *node, const Node *speedNode, MotionTriggerHandler *handler)
{
    parseTriggerHandler(node, handler);

    if (speedNode) {
        loadSetter(handler, &MotionTriggerHandler::setInputEventsToSample, speedNode->at("events"));
        if (const auto *thresholdNode = speedNode->at("swipe_threshold")) {
            handler->setSpeedThreshold(TriggerType::Swipe, thresholdNode->as<qreal>());
        }
    }
}

inline void parseMultiTouchMotionTriggerHandler(const Node *node, MultiTouchMotionTriggerHandler *handler)
{
    const auto *speedNode = node->mapAt("speed");
    parseMotionTriggerHandler(node, speedNode, handler);

    if (speedNode) {
        if (const auto *thresholdNode = speedNode->at("pinch_in_threshold")) {
            handler->setSpeedThreshold(TriggerType::Pinch, thresholdNode->as<qreal>(), static_cast<TriggerDirection>(PinchDirection::In));
        }
        if (const auto *thresholdNode = speedNode->at("pinch_out_threshold")) {
            handler->setSpeedThreshold(TriggerType::Pinch, thresholdNode->as<qreal>(), static_cast<TriggerDirection>(PinchDirection::Out));
        }
        if (const auto *thresholdNode = speedNode->at("rotate_threshold")) {
            handler->setSpeedThreshold(TriggerType::Rotate, thresholdNode->as<qreal>());
        }
    }
}

std::unique_ptr<TouchpadTriggerHandler> parseTouchpadTriggerHandler(const Node *node, InputDevice *device)
{
    auto handler = std::make_unique<TouchpadTriggerHandler>(device);
    parseMultiTouchMotionTriggerHandler(node, handler.get());
    loadSetter(static_cast<MotionTriggerHandler *>(handler.get()), &MotionTriggerHandler::setSwipeDeltaMultiplier, node->at("delta_multiplier"));
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
    result = std::make_unique<MouseTriggerHandler>();
    parseMotionTriggerHandler(node, node->mapAt("speed"), result.get());
}

template<>
void NodeParser<std::unique_ptr<PointerTriggerHandler>>::parse(const Node *node, std::unique_ptr<PointerTriggerHandler> &result)
{
    result = std::make_unique<PointerTriggerHandler>();
    parseTriggerHandler(node, result.get());
}

}
