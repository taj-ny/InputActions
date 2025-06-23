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

#include "interfaces/KWinInputEmitter.h"
#include "interfaces/KWinOnScreenMessageManager.h"
#include "interfaces/KWinPointer.h"
#include "interfaces/KWinSessionLock.hpp"
#include "interfaces/KWinWindowProvider.hpp"

#include <libinputactions/variables/VariableManager.h>

#include "effect/effecthandler.h"
#include "workspace.h"

#include <QDir>


Effect::Effect()
    : m_backend(new KWinInputBackend)
    , m_config(m_backend)
    , m_dbusInterface(&m_config)
{
    auto kwinPointer = std::make_shared<KWinPointer>();

    libinputactions::CursorShapeProvider::setInstance(kwinPointer);
    libinputactions::InputEmitter::setInstance(std::make_shared<KWinInputEmitter>());
    libinputactions::OnScreenMessageManager::setInstance(std::make_shared<KWinOnScreenMessageManager>());
    libinputactions::PointerPositionGetter::setInstance(kwinPointer);
    libinputactions::PointerPositionSetter::setInstance(kwinPointer);
    libinputactions::SessionLock::setInstance(std::make_shared<libinputactions::KWinSessionLock>());
    libinputactions::WindowProvider::setInstance(std::make_shared<KWinWindowProvider>());

    libinputactions::InputBackend::setInstance(std::unique_ptr<KWinInputBackend>(m_backend));

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
}

Effect::~Effect()
{
    if (KWin::input()) {
        KWin::input()->uninstallInputEventFilter(m_backend);
    }
}

void Effect::reconfigure(ReconfigureFlags flags)
{
    m_config.load();
}