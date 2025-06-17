/*
    Input Actions - Input handler that executes user-defined actions
    Copyright (C) 2024-2025 Marcin Woźniak

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

#include "Effect.h"
#include "input/KWinKeyboard.h"

#include "interfaces/KWinInputEmitter.h"
#include "interfaces/KWinPointer.h"
#include "interfaces/KWinSessionLock.hpp"
#include "interfaces/KWinWindowProvider.hpp"

#include <libinputactions/variables/VariableManager.h>
#include <libinputactions/yaml_convert.h>

#include "effect/effecthandler.h"
#include "workspace.h"

#include <QDir>

const static QString s_configFile = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/kwingestures.yml";

/**
 * Used to detect and prevent infinite compositor crash loops when loading the configuration.
 */
const static QString s_initFile = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/inputactions_init";

Effect::Effect()
    : m_backend(new KWinInputBackend)
{
    auto kwinPointer = std::make_shared<KWinPointer>();

    libinputactions::CursorShapeProvider::setInstance(kwinPointer);
    libinputactions::InputEmitter::setInstance(std::make_shared<KWinInputEmitter>());
    libinputactions::PointerPositionGetter::setInstance(kwinPointer);
    libinputactions::PointerPositionSetter::setInstance(kwinPointer);
    libinputactions::SessionLock::setInstance(std::make_shared<KWinSessionLock>());
    libinputactions::WindowProvider::setInstance(std::make_shared<KWinWindowProvider>());

    libinputactions::InputBackend::setInstance(std::unique_ptr<KWinInputBackend>(m_backend));
    libinputactions::Keyboard::setInstance(std::make_unique<KWinKeyboard>());

    // This should be moved to libinputactions eventually
    auto *variableManager = libinputactions::VariableManager::instance();
    variableManager->registerRemoteVariable<QString>("screen_name", [](auto &value) {
        if (const auto *output = KWin::workspace()->activeOutput()) {
            value = output->name();
        }
    });

#ifdef KWIN_6_2_OR_GREATER
    KWin::input()->installInputEventFilter(m_backend);
#else
    KWin::input()->prependInputEventFilter(m_backend);
#endif

    reconfigure(ReconfigureAll);

    if (!QFile::exists(s_configFile)) {
        QFile(s_configFile).open(QIODevice::WriteOnly);
    }
    m_configFileWatcher.addPath(s_configFile);
    m_configFileWatcher.addPath(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation));
    connect(&m_configFileWatcher, &QFileSystemWatcher::directoryChanged, this, &Effect::slotConfigDirectoryChanged);
    connect(&m_configFileWatcher, &QFileSystemWatcher::fileChanged, this, &Effect::slotConfigFileChanged);
}

Effect::~Effect()
{
    if (KWin::input()) {
        KWin::input()->uninstallInputEventFilter(m_backend);
    }
}

void Effect::slotConfigFileChanged()
{
    if (!m_configFileWatcher.files().contains(s_configFile)) {
        m_configFileWatcher.addPath(s_configFile);
    }

    if (m_autoReload) {
        reconfigure(ReconfigureAll);
    }
}

void Effect::slotConfigDirectoryChanged()
{
    if (!m_configFileWatcher.files().contains(s_configFile) && QFile::exists(s_configFile)) {
        m_configFileWatcher.addPath(s_configFile);
        if (m_autoReload) {
            reconfigure(ReconfigureAll);
        }
    }
}

void Effect::reconfigure(ReconfigureFlags flags)
{
    if (!QFile::exists(s_initFile)) {
        QFile(s_initFile).open(QIODevice::WriteOnly);
        try {
            const auto config = YAML::LoadFile(s_configFile.toStdString());
            m_autoReload = config["autoreload"].as<bool>(true);

            m_backend->reset();
            for (auto &eventHandler : config.as<std::vector<std::unique_ptr<InputEventHandler>>>()) {
                m_backend->addEventHandler(std::move(eventHandler));
            }
            const auto &devicesNode = config["touchpad"]["devices"];
            for (auto it = devicesNode.begin(); it != devicesNode.end(); it++) {
                m_backend->addCustomDeviceProperties(it->first.as<QString>(), it->second.as<InputDeviceProperties>());
            }

            auto *libevdev = static_cast<LibevdevComplementaryInputBackend *>(m_backend);
            if (const auto &pollingIntervalNode = config["__libevdev_polling_interval"]) {
                libevdev->setPollingInterval(pollingIntervalNode.as<uint32_t>());
            }
            if (const auto &enabledNode = config["__libevdev_enabled"]) {
                libevdev->setEnabled(enabledNode.as<bool>());
            }

            m_backend->initialize();
        } catch (const YAML::Exception &e) {
            qCritical(INPUTACTIONS).noquote() << QString("Failed to load configuration: %1 (line %2, column %3)")
                .arg(QString::fromStdString(e.msg), QString::number(e.mark.line), QString::number(e.mark.column));
        }
    } else {
        qCWarning(INPUTACTIONS) << "Configuration was not loaded automatically due to a crash.";
    }
    QFile::remove(s_initFile);
}