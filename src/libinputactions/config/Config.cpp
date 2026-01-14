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

#include "Config.h"
#include "../../common/ansi-escape-codes.h"
#include "Node.h"
#include "parsers/core.h"
#include "parsers/utils.h"
#include "yaml.h"
#include <QFile>
#include <QStandardPaths>
#include <libinputactions/globals.h>
#include <libinputactions/handlers/KeyboardTriggerHandler.h>
#include <libinputactions/handlers/MouseTriggerHandler.h>
#include <libinputactions/handlers/PointerTriggerHandler.h>
#include <libinputactions/handlers/TouchpadTriggerHandler.h>
#include <libinputactions/handlers/TouchscreenTriggerHandler.h>
#include <libinputactions/input/InputDeviceRule.h>
#include <libinputactions/input/backends/LibevdevComplementaryInputBackend.h>
#include <libinputactions/interfaces/ConfigProvider.h>
#include <libinputactions/interfaces/InputEmitter.h>
#include <libinputactions/interfaces/NotificationManager.h>

namespace InputActions
{

/**
 * Used to detect and prevent infinite compositor crash loops when loading the configuration.
 */
const static QString CRASH_PREVENTION_FILE = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/inputactions_init";

ConfigIssue::ConfigIssue(int32_t line, int32_t column, ConfigIssueSeverity severity, QString message)
    : m_line(line)
    , m_column(column)
    , m_severity(severity)
    , m_message(message)
{
}

ConfigParserException::ConfigParserException(const Node *node, const QString &message)
{
    node->disableMapAccessCheck();
    g_config->addIssue(node, ConfigIssueSeverity::Error, message);
}

bool Config::load(const QString &config, bool preventCrashLoops)
{
    if (!QFile::exists(CRASH_PREVENTION_FILE) || !preventCrashLoops) {
        std::ignore = QFile(CRASH_PREVENTION_FILE).open(QIODevice::WriteOnly);
        try {
            qCDebug(INPUTACTIONS, "Reloading config");
            g_config->m_issues.clear();

            auto newConfig = std::make_shared<Config>();
            Node root = YAML::Load(config.toStdString());

            loadSetter(newConfig, &Config::setAutoReload, root.at("autoreload").get());

            if (const auto notificationsNode = root.mapAt("notifications")) {
                loadSetter(newConfig, &Config::setSendNotificationOnError, notificationsNode->at("config_error").get());
            }

            const auto oldIssues = g_config->m_issues;
            g_config = newConfig;
            g_config->m_issues = oldIssues;

            std::unique_ptr<KeyboardTriggerHandler> keyboardTriggerHandler;
            loadMember(keyboardTriggerHandler, root.mapAt("keyboard").get());
            std::unique_ptr<MouseTriggerHandler> mouseTriggerHandler;
            loadMember(mouseTriggerHandler, root.mapAt("mouse").get());
            std::unique_ptr<PointerTriggerHandler> pointerTriggerHandler;
            loadMember(pointerTriggerHandler, root.mapAt("pointer").get());
            std::function<std::unique_ptr<TouchpadTriggerHandler>(InputDevice * device)> touchpadTriggerHandlerFactory;
            std::function<std::unique_ptr<TouchscreenTriggerHandler>(InputDevice * device)> touchscreenTriggerHandlerFactory;
            if (const auto touchpadNode = root.mapAt("touchpad")) {
                touchpadNode->disableMapAccessCheck("devices");

                touchpadTriggerHandlerFactory = [touchpadNode](auto *device) {
                    return parseTouchpadTriggerHandler(touchpadNode.get(), device);
                };
                touchpadTriggerHandlerFactory(nullptr); // Make sure it doesn't throw
            }
            if (const auto touchscreenNode = root.mapAt("touchscreen")) {
                touchscreenTriggerHandlerFactory = [touchscreenNode](auto *device) {
                    return parseTouchscreenTriggerHandler(touchscreenNode.get(), device);
                };
                touchscreenTriggerHandlerFactory(nullptr);
            }

            auto deviceRules = root.as<std::vector<InputDeviceRule>>();

            if (auto *libevdev = dynamic_cast<LibevdevComplementaryInputBackend *>(g_inputBackend.get())) {
                loadSetter(libevdev, &LibevdevComplementaryInputBackend::setEnabled, root.at("__libevdev_enabled").get());
            }

            g_inputBackend->reset();
            g_inputEmitter->reset(); // Okay because required keys are not cleared

            g_inputBackend->setKeyboardTriggerHandler(std::move(keyboardTriggerHandler));
            g_inputBackend->setMouseTriggerHandler(std::move(mouseTriggerHandler));
            g_inputBackend->setPointerTriggerHandler(std::move(pointerTriggerHandler));
            g_inputBackend->setTouchpadTriggerHandlerFactory(touchpadTriggerHandlerFactory);
            g_inputBackend->setTouchscreenTriggerHandlerFactory(touchscreenTriggerHandlerFactory);
            g_inputBackend->setDeviceRules(deviceRules);

            g_inputEmitter->initialize();
            g_inputBackend->initialize();
        } catch (const YAML::Exception &e) {
            g_config->addIssue(e.mark.line, e.mark.column, ConfigIssueSeverity::Error, QString("Syntax error: %1.").arg(e.what()));
        } catch (const ConfigParserException &) {
        }
    } else {
        g_config->addIssue(-1, -1, ConfigIssueSeverity::Error, "Configuration was not loaded automatically due to a crash.");
    }

    auto error = std::ranges::find_if(g_config->m_issues, [](const auto &issue) {
        return issue.severity() == ConfigIssueSeverity::Error;
    });
    if (error != g_config->m_issues.end()) {
        if (g_config->sendNotificationOnError()) {
            QString pos = "";
            if (error->line() != -1) {
                pos = QString::number(error->line()) + ":" + QString::number(error->column()) + ": ";
            }

            g_notificationManager->sendNotification("Failed to load configuration", pos + error->message());
        }
    }
    QFile::remove(CRASH_PREVENTION_FILE);
    return error == g_config->m_issues.end();
}

bool Config::load(bool preventCrashLoops)
{
    return load(g_configProvider->currentConfig(), preventCrashLoops);
}

void Config::addIssue(const Node *node, ConfigIssueSeverity severity, const QString &message)
{
    addIssue(node->line(), node->column(), severity, message);
}

void Config::addIssue(int32_t line, int32_t column, ConfigIssueSeverity severity, const QString &message)
{
    ConfigIssue issue(line, column, severity, message);
    if (std::ranges::contains(m_issues, issue)) {
        return;
    }

    // An error (exception) will result in most unused property issues generated after it being false-positives
    if (issue.severity() == ConfigIssueSeverity::UnusedProperty && std::ranges::any_of(m_issues, [](const auto &existingIssue) {
        return existingIssue.severity() == ConfigIssueSeverity::Error;
    })) {
        return;
    }

    auto pos = std::lower_bound(
        m_issues.begin(),
        m_issues.end(),
        std::tie(severity, line, column),
        [](const auto &issue, const auto &key) {
            // severity desc, line asc, column asc
            if (issue.severity() != std::get<0>(key))
                return issue.severity() > std::get<0>(key);
            if (issue.line() != std::get<1>(key))
                return issue.line() < std::get<1>(key);
            return issue.column() < std::get<2>(key);
        });

    m_issues.emplace(pos, line, column, severity, message);
}

QString Config::issuesToString(QString config) const
{
    QString result;

    if (config.isEmpty()) {
        config = g_configProvider->currentConfig();
    }
    const auto configLines = config.split("\n");

    const auto maxLineLength = QString::number(config.size() + 1).size();

    bool hasError{};
    for (const auto &issue : g_config->issues()) {
        if (issue.severity() == ConfigIssueSeverity::Error) {
            hasError = true;
        }

        const auto lineNumberPadding = [&maxLineLength](int32_t line) {
            return QString(" ").repeated(maxLineLength - QString::number(line + 1).size());
        };

        QString severityString;
        QString color;
        switch (issue.severity()) {
            case ConfigIssueSeverity::Deprecation:
                severityString = "deprecation";
                color = AnsiEscapeCode::Color::Blue;
                break;
            case ConfigIssueSeverity::UnusedProperty:
                severityString = "unused_property";
                color = AnsiEscapeCode::Color::Blue;
                break;
            case ConfigIssueSeverity::Warning:
                severityString = "warning";
                color = AnsiEscapeCode::Color::Yellow;
                break;
            case ConfigIssueSeverity::Error:
                severityString = "error";
                color = AnsiEscapeCode::Color::Red;
                break;
        }

        QString message = issue.message();
        const QString positionString = issue.line() == -1 ? "" : QString("%1:%2: ").arg(QString::number(issue.line() + 1), QString::number(issue.column() + 1));
        result += positionString + AnsiEscapeCode::Color::Bold + color + severityString + AnsiEscapeCode::Color::Reset + ": " + message + "\n";

        if (issue.line() != -1) {
            for (auto line = std::max(issue.line() - 5, 0); line < issue.line(); line++) {
                result += QString::number(line + 1) + lineNumberPadding(line) + " | " + configLines[line] + "\n";
            }
            result += QString::number(issue.line() + 1) + lineNumberPadding(issue.line()) + " | " + AnsiEscapeCode::Color::Bold + color + configLines[issue.line()] + AnsiEscapeCode::Color::Reset + "\n";

            bool skip = true;
            result += QString(" ").repeated(maxLineLength) + " | " + AnsiEscapeCode::Color::Bold + color;
            for (qsizetype i = 0; i < configLines[issue.line()].size(); i++) {
                const auto c = configLines[issue.line()][i];
                if (skip) {
                    if (c == ' ' || c == '\t') {
                        result += " ";
                        continue;
                    }
                    skip = false;
                }

                if (i == issue.column()) {
                    result += "^";
                } else  {
                    result += "~";
                }
            }
            result += AnsiEscapeCode::Color::Reset + "\n";

            for (auto line = issue.line() + 1; line < std::min(configLines.size(), static_cast<qsizetype>(issue.line() + 6)); line++) {
                result += QString::number(line + 1) + lineNumberPadding(line) + " | " + configLines[line] + "\n";
            }
        }
        result += "\n";
    }

    if (hasError) {
        result += "At least one error was found, which may have suppressed other issues. Run the command again after fixing it to ensure other problems are not missed.";
    }
    return result;
}

}