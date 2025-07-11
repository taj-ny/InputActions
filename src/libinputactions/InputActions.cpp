#include "InputActions.h"
#include "Config.h"
#include "input/Keyboard.h"
#include "input/backends/InputBackend.h"
#include "interfaces/CursorShapeProvider.h"
#include "interfaces/InputEmitter.h"
#include "interfaces/OnScreenMessageManager.h"
#include "interfaces/PointerPositionGetter.h"
#include "interfaces/PointerPositionSetter.h"
#include "interfaces/SessionLock.h"
#include "interfaces/WindowProvider.h"
#include "variables/VariableManager.h"

namespace libinputactions
{

InputActions::InputActions(std::unique_ptr<InputBackend> inputBackend)
{
    g_cursorShapeProvider = std::make_shared<CursorShapeProvider>();
    g_inputEmitter = std::make_shared<InputEmitter>();
    g_onScreenMessageManager = std::make_shared<OnScreenMessageManager>();
    g_pointerPositionGetter = std::make_shared<PointerPositionGetter>();
    g_pointerPositionSetter = std::make_shared<PointerPositionSetter>();
    g_sessionLock = std::make_shared<SessionLock>();
    g_windowProvider = std::make_shared<WindowProvider>();

    g_config = std::make_unique<Config>();
    g_inputBackend = std::move(inputBackend);
    g_keyboard = std::make_unique<Keyboard>();
    g_variableManager = std::make_unique<VariableManager>();
}

InputActions::~InputActions()
{
    g_cursorShapeProvider.reset();
    g_inputEmitter.reset();
    g_onScreenMessageManager.reset();
    g_pointerPositionGetter.reset();
    g_pointerPositionSetter.reset();
    g_sessionLock.reset();
    g_windowProvider.reset();

    g_config.reset();
    g_inputBackend.reset();
    g_keyboard.reset();
    g_variableManager.reset();
}

}