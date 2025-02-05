#include "effect.h"
#include "libgestures/yaml_convert.h"

#include <QDir>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(KWIN_GESTURES, "kwin_gestures", QtWarningMsg)

const QString configFile = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/kwingestures.yml";

Effect::Effect()
{
    libgestures::Input::setImplementation(new KWinInput);
    libgestures::WindowInfoProvider::setImplementation(new KWinWindowInfoProvider);
    registerBuiltinActions();

#ifdef KWIN_6_2_OR_GREATER
    KWin::input()->installInputEventFilter(m_inputEventFilter.get());
#else
    KWin::input()->prependInputEventFilter(m_inputEventFilter.get());
#endif

    reconfigure(ReconfigureAll);

    if (!QFile::exists(configFile)) {
        QFile(configFile).open(QIODevice::WriteOnly);
        configureWatcher();
    }

    connect(&m_configFileWatcher, &QFileSystemWatcher::directoryChanged, this, &Effect::slotConfigDirectoryChanged);
    connect(&m_configFileWatcher, &QFileSystemWatcher::fileChanged, this, &Effect::slotConfigFileChanged);
}

Effect::~Effect()
{
    if (KWin::input()) {
        KWin::input()->uninstallInputEventFilter(m_inputEventFilter.get());
    }
}

void Effect::slotConfigFileChanged()
{
    if (!m_configFileWatcher.files().contains(configFile)) {
        m_configFileWatcher.addPath(configFile);
    }

    reconfigure(ReconfigureAll);
}

void Effect::slotConfigDirectoryChanged()
{
    if (!m_configFileWatcher.files().contains(configFile) && QFile::exists(configFile)) {
        m_configFileWatcher.addPath(configFile);
        reconfigure(ReconfigureAll);
    }
}

void Effect::reconfigure(ReconfigureFlags flags)
{
    Q_UNUSED(flags)

    try {
        const auto config = YAML::LoadFile(configFile.toStdString());
        m_autoReload = config["autoreload"].as<bool>(true);
        auto gestureRecognizer = config["touchpad"].as<std::shared_ptr<libgestures::GestureRecognizer>>();
        m_inputEventFilter->setTouchpadGestureRecognizer(gestureRecognizer);
    } catch (const YAML::Exception &e) {
        qCritical(KWIN_GESTURES).noquote() << QStringLiteral("Failed to load configuration: ") + QString::fromStdString(e.msg)
                + " (line " + QString::number(e.mark.line) + ", column " + QString::number(e.mark.column) + ")";
        return;
    }

    if (m_autoReload) {
        configureWatcher();
    } else {
        m_configFileWatcher.removePaths(m_configFileWatcher.files() + m_configFileWatcher.directories());
    }
}

void Effect::configureWatcher()
{
    m_configFileWatcher.addPath(configFile);
    m_configFileWatcher.addPath(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation));
}

void Effect::registerBuiltinActions()
{
    auto collection = std::make_unique<libgestures::ActionCollection>();
    collection->setGestureTypes(libgestures::GestureType::Swipe);
    collection->setActionFactory([](auto &actions, auto &config) {
        auto action = std::make_unique<libgestures::PlasmaGlobalShortcutGestureAction>();
        action->setOn(config.isInstant ? libgestures::On::Begin : libgestures::On::End);
        action->setComponent("kwin");
        action->setShortcut("Window Maximize");
        actions.push_back(std::move(action));
    });
    libgestures::ActionCollection::registerCollection("maximize", std::move(collection));

    collection = std::make_unique<libgestures::ActionCollection>();
    collection->setGestureTypes(libgestures::GestureType::Swipe);
    collection->setActionFactory([](auto &actions, auto &config) {
        libgestures::InputAction input;

        auto action = std::make_unique<libgestures::InputGestureAction>();
        action->setOn(libgestures::On::Begin);
        input.keyboardPress.push_back(KEY_LEFTMETA);
        input.mousePress.push_back(BTN_LEFT);
        action->setSequence({input});
        actions.push_back(std::move(action));

        input = {};
        action = std::make_unique<libgestures::InputGestureAction>();
        action->setOn(libgestures::On::Update);
        input.mouseMoveRelativeByDelta = true;
        action->setSequence({input});
        actions.push_back(std::move(action));

        input = {};
        action = std::make_unique<libgestures::InputGestureAction>();
        action->setOn(libgestures::On::EndOrCancel);
        input.keyboardRelease.push_back(KEY_LEFTMETA);
        input.mouseRelease.push_back(BTN_LEFT);
        action->setSequence({input});
        actions.push_back(std::move(action));
    });
    libgestures::ActionCollection::registerCollection("drag_window", std::move(collection));
}