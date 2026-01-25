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
#include "input/backends/InputBackend.h"
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
#include <libinputactions/config/ConfigIssueManager.h>
#include <libinputactions/config/ConfigIssue.h>
#include <libinputactions/config/Node.h>
#include <libinputactions/config/UnusedNodePropertyTracker.h>
#include <libinputactions/handlers/KeyboardTriggerHandler.h>
#include <libinputactions/handlers/MouseTriggerHandler.h>
#include <libinputactions/handlers/PointerTriggerHandler.h>
#include <libinputactions/handlers/TouchpadTriggerHandler.h>
#include <libinputactions/handlers/TouchscreenTriggerHandler.h>
#include <libinputactions/input/backends/InputBackend.h>
#include <libinputactions/input/InputDeviceRule.h>
#include <libinputactions/input/KeyboardKey.h>
#include <libinputactions/input/MouseButton.h>
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

class ConfigParser;

static Value<std::any> parseAny(const Node *node, const std::type_index &type)
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
    throw InvalidValueConfigException(node->position(), "Unexpected type.");
}

/**
 * @param arguments x and y, size must be 2
 */
static QPointF parseMouseInputActionPoint(const Node *node, const QStringList &arguments)
{
    if (arguments.length() != 2) {
        throw InvalidValueConfigException(node->position(), "Invalid point: wrong argument count.");
    }

    bool ok1{};
    bool ok2{};
    const QPointF point(arguments[0].toDouble(&ok1), arguments[1].toDouble(&ok2));

    if (!ok1) {
        throw InvalidValueConfigException(node->position(), QString("Invalid argument 1 '%1': not a number.").arg(arguments[0]));
    }
    if (!ok2) {
        throw InvalidValueConfigException(node->position(), QString("Invalid argument 2 '%1': not a number.").arg(arguments[1]));
    }

    return point;
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
        throw InvalidVariableConfigException(node->position(), variableName);
    }

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
            throw InvalidValueConfigException(node->position(), QString("Invalid operator '%1'.").arg(operatorRaw));
        }
        comparisonOperator = operators.at(operatorRaw);

        const auto rightRaw = raw.mid(secondSpace + 1);
        auto rightNode = node->substringNode(rightRaw);

        if (comparisonOperator == ComparisonOperator::Regex) {
            rightNode.as<QRegularExpression>();
        }

        if (!isTypeFlags(variable->type()) && rightNode.isSequence()) {
            for (auto &child : rightNode.sequenceChildren()) {
                child->setPosition(node->position());
                right.push_back(parseAny(child.get(), variable->type()));
            }
        } else if (rightRaw.contains(';')) {
            const auto split = rightRaw.split(';');
            const auto minNode = node->substringNode(split[0]);
            const auto maxNode = node->substringNode(split[1]);

            right.push_back(parseAny(&minNode, variable->type()));
            right.push_back(parseAny(&maxNode, variable->type()));
        } else {
            right.push_back(parseAny(&rightNode, variable->type()));
        }
    }

    auto condition = std::make_shared<VariableCondition>(variableName, right, comparisonOperator);
    condition->setNegate(negate);
    return condition;
}

template<>
void NodeParser<std::shared_ptr<Action>>::parse(const Node *node, std::shared_ptr<Action> &result)
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
            throw InvalidValueConfigException(node->position(), "Invalid Plasma shortcut format");
        }
        result = std::make_shared<PlasmaGlobalShortcutAction>(split[0], split[1]);
    } else if (const auto sleepActionNode = node->at("sleep")) {
        result = std::make_shared<SleepAction>(sleepActionNode->as<std::chrono::milliseconds>());
    } else if (const auto oneNode = node->at("one")) {
        result = std::make_shared<ActionGroup>(oneNode->as<std::vector<std::shared_ptr<Action>>>(), ActionGroup::ExecutionMode::First);
    } else {
        throw InvalidValueConfigException(node->position(), "Action is missing a required property that determines its type.");
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
        std::shared_ptr<const Node> groupChildren;
        if (const auto allNode = node->at("all")) {
            groupMode = ConditionGroupMode::All;
            groupChildren = allNode;
        } else if (const auto anyNode = node->at("any")) {
            groupMode = ConditionGroupMode::Any;
            groupChildren = anyNode;
        } else if (const auto noneNode = node->at("none")) {
            groupMode = ConditionGroupMode::None;
            groupChildren = noneNode;
        }
        if (groupMode) {
            auto group = std::make_shared<ConditionGroup>(*groupMode);
            for (const auto &child : groupChildren->sequenceChildren()) {
                group->append(parseCondition(child.get(), variableManager));
            }
            return group;
        }

        if (isLegacy(node)) {
            g_configIssueManager->addIssue(DeprecatedFeatureConfigIssue(node->position(), DeprecatedFeature::LegacyConditions));

            auto group = std::make_shared<ConditionGroup>();
            QStringList negate;
            loadMember(negate, node->at("negate").get());

            if (const auto windowClassNode = node->at("window_class")) {
                const auto value = Value(windowClassNode->as<QString>());
                auto classGroup = std::make_shared<ConditionGroup>(ConditionGroupMode::Any);
                classGroup->append(std::make_shared<VariableCondition>("window_class", value, ComparisonOperator::Regex));
                classGroup->append(std::make_shared<VariableCondition>("window_name", value, ComparisonOperator::Regex));
                classGroup->setNegate(negate.contains("window_class"));
                group->append(classGroup);
            }
            if (const auto windowStateNode = node->at("window_state")) {
                QStringList value;
                loadMember(value, windowStateNode.get());

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
            return isLegacy(node.get());
        };

        const auto children = node->sequenceChildren(true);
        const auto hasLegacyCondition = std::ranges::any_of(children, legacyPred);
        if (hasLegacyCondition && !std::ranges::all_of(children, legacyPred)) {
            throw InvalidValueConfigException(node->position(), "Mixing legacy and normal conditions is not allowed.");
        }

        auto group = std::make_shared<ConditionGroup>(hasLegacyCondition ? ConditionGroupMode::Any : ConditionGroupMode::All);
        for (const auto &child : node->sequenceChildren()) {
            group->append(parseCondition(child.get(), variableManager));
        }
        return group;
    }

    if (node->isScalar()) {
        // Hack to load negated conditions without forcing users to quote the entire thing
        auto conditionNode = node->clone();
        const auto tag = node->tag();
        if (tag != "!" && tag.startsWith('!')) {
            conditionNode.raw() = QString("%1 %2").arg(tag, node->as<QString>()).trimmed().toStdString();
        }

        const auto raw = conditionNode.as<QString>();
        if (raw.startsWith("$") || raw.startsWith("!$")) {
            return parseVariableCondition(&conditionNode, variableManager);
        }
    }

    return {};
}

template<>
void NodeParser<std::shared_ptr<Condition>>::parse(const Node *node, std::shared_ptr<Condition> &condition)
{
    condition = parseCondition(node);
}

template<>
void NodeParser<std::vector<InputAction::Item>>::parse(const Node *node, std::vector<InputAction::Item> &result)
{
    for (const auto &deviceNode : node->sequenceChildren()) {
        if (const auto keyboardNode = deviceNode->at("keyboard")) {
            for (const auto &actionNode : keyboardNode->sequenceChildren()) {
                if (actionNode->isMap()) {
                    if (const auto textNode = actionNode->at("text")) {
                        result.push_back({
                            .keyboardText = textNode->as<Value<QString>>(),
                        });
                    }
                } else {
                    const auto actionRaw = actionNode->as<QString>().toUpper();
                    if (actionRaw.startsWith("+") || actionRaw.startsWith("-")) {
                        const auto key = actionNode->substringNode(actionRaw.mid(1)).as<KeyboardKey>();

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
                            throw InvalidValueConfigException(actionNode->position(), "Syntax error: found at least two '+' characters next to each other with no key in between.");
                        }
                        if (actionRaw.endsWith("+")) {
                            throw InvalidValueConfigException(actionNode->position(), "Syntax error: found trailing '+' character with no key after.");
                        }

                        std::vector<uint32_t> keys;
                        for (const auto &keyRaw : actionRaw.split("+")) {
                            keys.push_back(actionNode->substringNode(keyRaw).as<KeyboardKey>());
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
        } else if (const auto mouseNode = deviceNode->at("mouse")) {
            for (const auto &actionNode : mouseNode->sequenceChildren()) {
                const auto actionRaw = actionNode->as<QString>();
                if (actionRaw.contains("  ")) {
                    throw InvalidValueConfigException(actionNode->position(), "Syntax error: found at least two space characters next to each other.");
                }

                const auto split = actionRaw.split(' ');
                const auto action = split[0].toUpper();
                const auto arguments = split.mid(1);

                if (action.startsWith("+") || action.startsWith("-")) {
                    const auto button = actionNode->substringNode(action.mid(1)).as<MouseButton>();

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
                        .mouseMoveRelative = parseMouseInputActionPoint(actionNode.get(), arguments),
                    });
                } else if (action.startsWith("MOVE_TO")) {
                    result.push_back({
                        .mouseMoveAbsolute = parseMouseInputActionPoint(actionNode.get(), arguments),
                    });
                } else if (action.startsWith("WHEEL")) {
                    result.push_back({
                        .mouseAxis = parseMouseInputActionPoint(actionNode.get(), arguments),
                    });
                } else {
                    if (actionRaw.contains("++")) {
                        throw InvalidValueConfigException(actionNode->position(), "Syntax error: found at least two '+' characters next to each other with no button in between.");
                    }
                    if (actionRaw.endsWith("+")) {
                        throw InvalidValueConfigException(actionNode->position(), "Syntax error: found trailing '+' character with no button after.");
                    }

                    std::vector<uint32_t> buttons;
                    for (const auto &buttonRaw : actionRaw.split("+")) {
                        buttons.push_back(actionNode->substringNode(buttonRaw).as<MouseButton>());
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
            throw InvalidValueConfigException(deviceNode->position(), "Invalid device type.");
        }
    }
}

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
            if (const auto conditionsNode = ruleNode->at("conditions")) {
                rule.setCondition(parseCondition(conditionsNode.get(), g_inputBackend->deviceRulesVariableManager()));
            }
            loadSetter(rule, &InputDeviceRule::setProperties, ruleNode.get());
            result.push_back(std::move(rule));
        }
    }

    // Legacy
    if (const auto touchpadNode = node->mapAt("touchpad")) {
        touchpadNode->unusedPropertyTracker()->setEnabled(false); // Handled when parsing trigger handler

        if (const auto devicesNode = touchpadNode->mapAt("devices")) {
            g_configIssueManager->addIssue(DeprecatedFeatureConfigIssue(devicesNode->position(), DeprecatedFeature::TouchpadDevicesNode));
            devicesNode->unusedPropertyTracker()->setEnabled(false);

            for (const auto &[key, value] : devicesNode->mapChildren()) {
                InputDeviceRule rule;
                rule.setCondition(std::make_shared<VariableCondition>("name", Value(key->as<QString>()), ComparisonOperator::EqualTo));
                loadSetter(rule, &InputDeviceRule::setProperties, value.get());
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

    throw InvalidValueConfigException(node->position(), QString("Invalid keyboard key '%1'.").arg(raw));
}

template<>
void NodeParser<MouseButton>::parse(const Node *node, MouseButton &result)
{
    const auto raw = node->as<QString>();
    if (const auto button = MouseButton::fromString(raw.toUpper())) {
        result = button.value();
        return;
    }

    throw InvalidValueConfigException(node->position(), QString("Invalid mouse button '%1'.").arg(raw));
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
        const auto raw = node->as<QString>().replace(" ", "");
        if (!raw.contains("-")) {
            result = Range<T>(node->as<T>(), {});
            return;
        }

        const auto split = raw.split("-", Qt::SkipEmptyParts);
        if (split.count() != 2) {
            throw InvalidValueConfigException(node->position(), "Invalid range, correct format is 'min-max'.");
        }

        result = Range<T>(node->substringNode(split[0]).as<T>(), node->substringNode(split[1]).as<T>());
    }
};

template<>
void NodeParser<std::unique_ptr<Trigger>>::parse(const Node *node, std::unique_ptr<Trigger> &result)
{
    const auto typeNode = node->at("type", true);
    auto type = typeNode->as<QString>();

    if (type == "circle") {
        result = std::make_unique<DirectionalMotionTrigger>(TriggerType::Circle,
                                                            static_cast<TriggerDirection>(node->at("direction", true)->as<RotateDirection>()));
    } else if (type == "click") {
        result = std::make_unique<Trigger>(TriggerType::Click);
    } else if (type == "hold" || type == "press") {
        auto *pressTrigger = new PressTrigger;
        loadSetter(pressTrigger, &PressTrigger::setInstant, node->at("instant").get());
        result.reset(pressTrigger);
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
        result = std::make_unique<StrokeTrigger>(node->at("strokes", true)->asSequence().as<std::vector<Stroke>>());
    } else if (type == "swipe") {
        result = std::make_unique<DirectionalMotionTrigger>(TriggerType::Swipe,
                                                            static_cast<TriggerDirection>(node->at("direction", true)->as<SwipeDirection>()));
    } else if (type == "tap") {
        result = std::make_unique<Trigger>(TriggerType::Tap);
    } else if (type == "wheel") {
        result = std::make_unique<WheelTrigger>(static_cast<TriggerDirection>(node->at("direction", true)->as<SwipeDirection>()));
    } else {
        throw InvalidValueConfigException(typeNode->position(), QString("Invalid trigger type '%1'.").arg(type));
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
    if (const auto modifiersNode = node->at("keyboard_modifiers")) {
        std::optional<Qt::KeyboardModifiers> modifiers;
        if (modifiersNode->isSequence()) {
            modifiers = modifiersNode->as<Qt::KeyboardModifiers>();
        } else {
            const auto modifierMatchingMode = modifiersNode->as<QString>();
            if (modifierMatchingMode == "none") {
                modifiers = Qt::KeyboardModifier::NoModifier;
            } else if (modifierMatchingMode != "any") {
                throw InvalidValueConfigException(node->position(), "Invalid keyboard modifier.");
            }
        }

        if (modifiers) {
            conditionGroup->append(std::make_shared<VariableCondition>(BuiltinVariables::KeyboardModifiers,
                                                                       Value<Qt::KeyboardModifiers>(modifiers.value()),
                                                                       ComparisonOperator::EqualTo));
        }
    }
    if (const auto conditionsNode = node->at("conditions")) {
        conditionGroup->append(conditionsNode->as<std::shared_ptr<Condition>>());
    }

    if (conditionGroup->conditions().size() == 1) {
        result->setActivationCondition(conditionGroup->conditions()[0]);
    } else if (conditionGroup->conditions().size()) {
        result->setActivationCondition(conditionGroup);
    }

    bool accelerated{};
    loadMember(accelerated, node->at("accelerated").get());

    if (const auto actionsNode = node->at("actions")) {
        for (const auto &actionNode : actionsNode->sequenceChildren()) {
            auto action = actionNode->as<std::unique_ptr<TriggerAction>>();
            if (dynamic_cast<StrokeTrigger *>(result.get()) && action->on() != On::End) {
                throw InvalidValueConfigException(actionNode->position(), "Stroke triggers only support 'on: end' actions.");
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
    for (const auto &triggerNode : node->sequenceChildren()) {
        if (auto subTriggersNode = triggerNode->at("gestures")) {
            triggerNode->unusedPropertyTracker()->setEnabled(false);
            subTriggersNode->unusedPropertyTracker()->setEnabled(false);

            // Trigger group
            for (const auto &subTriggerNode : subTriggersNode->sequenceChildren()) {
                subTriggerNode->unusedPropertyTracker()->setEnabled(false);

                // Not cloned to preserve the mark. Touchpads and touchscreens require parsing the trigger list node multiple times.
                auto rawNode = subTriggerNode->raw();

                std::shared_ptr<Condition> groupCondition;
                for (const auto &[key, value] : triggerNode->mapChildren()) {
                    const auto name = key->as<QString>();

                    if (name == "conditions") {
                        groupCondition = triggerNode->at("conditions")->as<std::shared_ptr<Condition>>();
                    } else if (name != "gestures") {
                        rawNode[key->raw()] = value->raw();
                    }
                }

                Node processedNode(rawNode);
                for (auto &trigger : processedNode.asSequence().as<std::vector<std::unique_ptr<Trigger>>>()) {
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
    result = std::make_unique<TriggerAction>(node->as<std::shared_ptr<Action>>());

    loadSetter(result, &TriggerAction::setOn, node->at("on").get());

    if (const auto intervalNode = node->at("interval")) {
        if (result->on() != On::Update && result->on() != On::Tick) {
            throw InvalidValueConfigException(intervalNode->position(), "Intervals can only be set on update and tick actions.");
        }
        loadSetter(result, &TriggerAction::setInterval, intervalNode.get());
    }

    if (const auto thresholdNode = node->at("threshold")) {
        if (result->on() == On::Begin) {
            throw InvalidValueConfigException(thresholdNode->position(), "Thresholds cannot be set on begin actions.");
        }
        loadSetter(result, &TriggerAction::setThreshold, thresholdNode.get());
    }
}

template<>
void NodeParser<Stroke>::parse(const Node *node, Stroke &result)
{
    const auto bytes = QByteArray::fromBase64(node->as<QString>().toUtf8());
    if (bytes.size() % 4 != 0) {
        throw InvalidValueConfigException(node->position(), "Invalid stroke.");
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
    for (auto &trigger : node->at("gestures", true)->as<std::vector<std::unique_ptr<Trigger>>>()) {
        handler->addTrigger(std::move(trigger));
    }
    loadSetter(handler, &TriggerHandler::setTimedTriggerUpdateDelta, node->at("__time_delta").get());
}

inline void parseMotionTriggerHandler(const Node *node, const std::shared_ptr<const Node> &speedNode, MotionTriggerHandler *handler)
{
    parseTriggerHandler(node, handler);

    if (speedNode) {
        loadSetter(handler, &MotionTriggerHandler::setInputEventsToSample, speedNode->at("events").get());
        if (auto thresholdNode = speedNode->at("swipe_threshold")) {
            handler->setSpeedThreshold(TriggerType::Swipe, thresholdNode->as<qreal>());
        }
    }
}

inline void parseMultiTouchMotionTriggerHandler(const Node *node, MultiTouchMotionTriggerHandler *handler)
{
    const auto speedNode = node->mapAt("speed");
    parseMotionTriggerHandler(node, speedNode, handler);

    if (speedNode) {
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
    parseMotionTriggerHandler(node, node->mapAt("speed"), result.get());

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
