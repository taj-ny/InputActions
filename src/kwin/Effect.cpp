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
#include "effect/effecthandler.h"
#include "input/KWinInputBackend.h"
#include "interfaces/KWinInputEmitter.h"
#include "interfaces/KWinOnScreenMessageManager.h"
#include "interfaces/KWinPointer.h"
#include "interfaces/KWinSessionLock.h"
#include "interfaces/KWinWindowProvider.h"
#include "workspace.h"
#include <QDir>
#include <libinputactions/Config.h>
#include <libinputactions/variables/VariableManager.h>

using namespace libinputactions;

Effect::Effect()
    : InputActions(std::make_unique<KWinInputBackend>())
{
    auto pointer = std::make_shared<KWinPointer>();
    g_cursorShapeProvider = pointer;
    g_inputEmitter = std::make_shared<KWinInputEmitter>();
    g_onScreenMessageManager = std::make_shared<KWinOnScreenMessageManager>();
    g_pointerPositionGetter = pointer;
    g_pointerPositionSetter = pointer;
    g_sessionLock = std::make_shared<KWinSessionLock>();
    g_windowProvider = std::make_shared<KWinWindowProvider>();

    // Some of this should be moved to libinputactions eventually
    g_variableManager->registerRemoteVariable<bool>("plasma_overview_active", [](auto &value) {
        // Overview is a plugin and headers are not provided, I think the best way right now is to check for the presence of a QObject property, as the effect
        // does fortunately have them.
        if (const auto *effect = dynamic_cast<KWin::Effect *>(KWin::effects->activeFullScreenEffect())) {
            value = effect->property("overviewGestureInProgress").isValid();
        } else {
            value = false;
        }
    });
    g_variableManager->registerRemoteVariable<QString>("screen_name", [](auto &value) {
        if (const auto *output = KWin::workspace()->activeOutput()) {
            value = output->name();
        }
    });

    g_config->load(true);
}

void Effect::reconfigure(ReconfigureFlags flags)
{
    g_config->load();
}