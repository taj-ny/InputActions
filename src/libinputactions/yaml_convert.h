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

#pragma once

#include "yaml-cpp/yaml.h"
#include <QRegularExpression>
#include <QVector>
#include <libinputactions/Expression.cpp>
#include <libinputactions/Value.h>
#include <libinputactions/actions/ActionGroup.h>
#include <libinputactions/actions/CommandAction.h>
#include <libinputactions/actions/InputAction.h>
#include <libinputactions/actions/PlasmaGlobalShortcutAction.h>
#include <libinputactions/actions/TriggerAction.h>
#include <libinputactions/conditions/ConditionGroup.h>
#include <libinputactions/conditions/VariableCondition.h>
#include <libinputactions/handlers/KeyboardTriggerHandler.h>
#include <libinputactions/handlers/MouseTriggerHandler.h>
#include <libinputactions/handlers/TouchpadTriggerHandler.h>
#include <libinputactions/input/InputEventHandler.h>
#include <libinputactions/input/Keyboard.h>
#include <libinputactions/interfaces/CursorShapeProvider.h>
#include <libinputactions/triggers/KeyboardShortcutTrigger.h>
#include <libinputactions/triggers/PressTrigger.h>
#include <libinputactions/triggers/StrokeTrigger.h>
#include <libinputactions/triggers/WheelTrigger.h>
#include <libinputactions/variables/Variable.h>
#include <libinputactions/variables/VariableManager.h>
#include <unordered_set>

// Keep s_keyboard and s_mouse at the top, the documentation links to them.

static const std::unordered_map<QString, uint32_t> s_keyboard = {
    {"RESERVED", KEY_RESERVED},
    {"ESC", KEY_ESC},
    {"1", KEY_1},
    {"2", KEY_2},
    {"3", KEY_3},
    {"4", KEY_4},
    {"5", KEY_5},
    {"6", KEY_6},
    {"7", KEY_7},
    {"8", KEY_8},
    {"9", KEY_9},
    {"0", KEY_0},
    {"MINUS", KEY_MINUS},
    {"EQUAL", KEY_EQUAL},
    {"BACKSPACE", KEY_BACKSPACE},
    {"TAB", KEY_TAB},
    {"Q", KEY_Q},
    {"W", KEY_W},
    {"E", KEY_E},
    {"R", KEY_R},
    {"T", KEY_T},
    {"Y", KEY_Y},
    {"U", KEY_U},
    {"I", KEY_I},
    {"O", KEY_O},
    {"P", KEY_P},
    {"LEFTBRACE", KEY_LEFTBRACE},
    {"RIGHTBRACE", KEY_RIGHTBRACE},
    {"ENTER", KEY_ENTER},
    {"LEFTCTRL", KEY_LEFTCTRL},
    {"A", KEY_A},
    {"S", KEY_S},
    {"D", KEY_D},
    {"F", KEY_F},
    {"G", KEY_G},
    {"H", KEY_H},
    {"J", KEY_J},
    {"K", KEY_K},
    {"L", KEY_L},
    {"SEMICOLON", KEY_SEMICOLON},
    {"APOSTROPHE", KEY_APOSTROPHE},
    {"GRAVE", KEY_GRAVE},
    {"LEFTSHIFT", KEY_LEFTSHIFT},
    {"BACKSLASH", KEY_BACKSLASH},
    {"Z", KEY_Z},
    {"X", KEY_X},
    {"C", KEY_C},
    {"V", KEY_V},
    {"B", KEY_B},
    {"N", KEY_N},
    {"M", KEY_M},
    {"COMMA", KEY_COMMA},
    {"DOT", KEY_DOT},
    {"SLASH", KEY_SLASH},
    {"RIGHTSHIFT", KEY_RIGHTSHIFT},
    {"KPASTERISK", KEY_KPASTERISK},
    {"LEFTALT", KEY_LEFTALT},
    {"SPACE", KEY_SPACE},
    {"CAPSLOCK", KEY_CAPSLOCK},
    {"F1", KEY_F1},
    {"F2", KEY_F2},
    {"F3", KEY_F3},
    {"F4", KEY_F4},
    {"F5", KEY_F5},
    {"F6", KEY_F6},
    {"F7", KEY_F7},
    {"F8", KEY_F8},
    {"F9", KEY_F9},
    {"F10", KEY_F10},
    {"NUMLOCK", KEY_NUMLOCK},
    {"SCROLLLOCK", KEY_SCROLLLOCK},
    {"KP7", KEY_KP7},
    {"KP8", KEY_KP8},
    {"KP9", KEY_KP9},
    {"KPMINUS", KEY_KPMINUS},
    {"KP4", KEY_KP4},
    {"KP5", KEY_KP5},
    {"KP6", KEY_KP6},
    {"KPPLUS", KEY_KPPLUS},
    {"KP1", KEY_KP1},
    {"KP2", KEY_KP2},
    {"KP3", KEY_KP3},
    {"KP0", KEY_KP0},
    {"KPDOT", KEY_KPDOT},
    {"ZENKAKUHANKAKU", KEY_ZENKAKUHANKAKU},
    {"102ND", KEY_102ND},
    {"F11", KEY_F11},
    {"F12", KEY_F12},
    {"RO", KEY_RO},
    {"KATAKANA", KEY_KATAKANA},
    {"HIRAGANA", KEY_HIRAGANA},
    {"HENKAN", KEY_HENKAN},
    {"KATAKANAHIRAGANA", KEY_KATAKANAHIRAGANA},
    {"MUHENKAN", KEY_MUHENKAN},
    {"KPJPCOMMA", KEY_KPJPCOMMA},
    {"KPENTER", KEY_KPENTER},
    {"RIGHTCTRL", KEY_RIGHTCTRL},
    {"KPSLASH", KEY_KPSLASH},
    {"SYSRQ", KEY_SYSRQ},
    {"RIGHTALT", KEY_RIGHTALT},
    {"LINEFEED", KEY_LINEFEED},
    {"HOME", KEY_HOME},
    {"UP", KEY_UP},
    {"PAGEUP", KEY_PAGEUP},
    {"LEFT", KEY_LEFT},
    {"RIGHT", KEY_RIGHT},
    {"END", KEY_END},
    {"DOWN", KEY_DOWN},
    {"PAGEDOWN", KEY_PAGEDOWN},
    {"INSERT", KEY_INSERT},
    {"DELETE", KEY_DELETE},
    {"MACRO", KEY_MACRO},
    {"MUTE", KEY_MUTE},
    {"VOLUMEDOWN", KEY_VOLUMEDOWN},
    {"VOLUMEUP", KEY_VOLUMEUP},
    {"POWER", KEY_POWER},
    {"KPEQUAL", KEY_KPEQUAL},
    {"KPPLUSMINUS", KEY_KPPLUSMINUS},
    {"PAUSE", KEY_PAUSE},
    {"SCALE", KEY_SCALE},
    {"KPCOMMA", KEY_KPCOMMA},
    {"HANGEUL", KEY_HANGEUL},
    {"HANJA", KEY_HANJA},
    {"YEN", KEY_YEN},
    {"LEFTMETA", KEY_LEFTMETA},
    {"RIGHTMETA", KEY_RIGHTMETA},
    {"COMPOSE", KEY_COMPOSE},
    {"STOP", KEY_STOP},
    {"AGAIN", KEY_AGAIN},
    {"PROPS", KEY_PROPS},
    {"UNDO", KEY_UNDO},
    {"FRONT", KEY_FRONT},
    {"COPY", KEY_COPY},
    {"OPEN", KEY_OPEN},
    {"PASTE", KEY_PASTE},
    {"FIND", KEY_FIND},
    {"CUT", KEY_CUT},
    {"HELP", KEY_HELP},
    {"MENU", KEY_MENU},
    {"CALC", KEY_CALC},
    {"SETUP", KEY_SETUP},
    {"SLEEP", KEY_SLEEP},
    {"WAKEUP", KEY_WAKEUP},
    {"FILE", KEY_FILE},
    {"SENDFILE", KEY_SENDFILE},
    {"DELETEFILE", KEY_DELETEFILE},
    {"XFER", KEY_XFER},
    {"PROG1", KEY_PROG1},
    {"PROG2", KEY_PROG2},
    {"WWW", KEY_WWW},
    {"MSDOS", KEY_MSDOS},
    {"COFFEE", KEY_COFFEE},
    {"ROTATE_DISPLAY", KEY_ROTATE_DISPLAY},
    {"CYCLEWINDOWS", KEY_CYCLEWINDOWS},
    {"MAIL", KEY_MAIL},
    {"BOOKMARKS", KEY_BOOKMARKS},
    {"COMPUTER", KEY_COMPUTER},
    {"BACK", KEY_BACK},
    {"FORWARD", KEY_FORWARD},
    {"CLOSECD", KEY_CLOSECD},
    {"EJECTCD", KEY_EJECTCD},
    {"EJECTCLOSECD", KEY_EJECTCLOSECD},
    {"NEXTSONG", KEY_NEXTSONG},
    {"PLAYPAUSE", KEY_PLAYPAUSE},
    {"PREVIOUSSONG", KEY_PREVIOUSSONG},
    {"STOPCD", KEY_STOPCD},
    {"RECORD", KEY_RECORD},
    {"REWIND", KEY_REWIND},
    {"PHONE", KEY_PHONE},
    {"ISO", KEY_ISO},
    {"CONFIG", KEY_CONFIG},
    {"HOMEPAGE", KEY_HOMEPAGE},
    {"REFRESH", KEY_REFRESH},
    {"EXIT", KEY_EXIT},
    {"MOVE", KEY_MOVE},
    {"EDIT", KEY_EDIT},
    {"SCROLLUP", KEY_SCROLLUP},
    {"SCROLLDOWN", KEY_SCROLLDOWN},
    {"KPLEFTPAREN", KEY_KPLEFTPAREN},
    {"KPRIGHTPAREN", KEY_KPRIGHTPAREN},
    {"NEW", KEY_NEW},
    {"REDO", KEY_REDO},
    {"F13", KEY_F13},
    {"F14", KEY_F14},
    {"F15", KEY_F15},
    {"F16", KEY_F16},
    {"F17", KEY_F17},
    {"F18", KEY_F18},
    {"F19", KEY_F19},
    {"F20", KEY_F20},
    {"F21", KEY_F21},
    {"F22", KEY_F22},
    {"F23", KEY_F23},
    {"F24", KEY_F24},
    {"PLAYCD", KEY_PLAYCD},
    {"PAUSECD", KEY_PAUSECD},
    {"PROG3", KEY_PROG3},
    {"PROG4", KEY_PROG4},
    {"ALL_APPLICATIONS", KEY_ALL_APPLICATIONS},
    {"SUSPEND", KEY_SUSPEND},
    {"CLOSE", KEY_CLOSE},
    {"PLAY", KEY_PLAY},
    {"FASTFORWARD", KEY_FASTFORWARD},
    {"BASSBOOST", KEY_BASSBOOST},
    {"PRINT", KEY_PRINT},
    {"HP", KEY_HP},
    {"CAMERA", KEY_CAMERA},
    {"SOUND", KEY_SOUND},
    {"QUESTION", KEY_QUESTION},
    {"EMAIL", KEY_EMAIL},
    {"CHAT", KEY_CHAT},
    {"SEARCH", KEY_SEARCH},
    {"CONNECT", KEY_CONNECT},
    {"FINANCE", KEY_FINANCE},
    {"SPORT", KEY_SPORT},
    {"SHOP", KEY_SHOP},
    {"ALTERASE", KEY_ALTERASE},
    {"CANCEL", KEY_CANCEL},
    {"BRIGHTNESSDOWN", KEY_BRIGHTNESSDOWN},
    {"BRIGHTNESSUP", KEY_BRIGHTNESSUP},
    {"MEDIA", KEY_MEDIA},
    {"SWITCHVIDEOMODE", KEY_SWITCHVIDEOMODE},
    {"KBDILLUMTOGGLE", KEY_KBDILLUMTOGGLE},
    {"KBDILLUMDOWN", KEY_KBDILLUMDOWN},
    {"KBDILLUMUP", KEY_KBDILLUMUP},
    {"SEND", KEY_SEND},
    {"REPLY", KEY_REPLY},
    {"FORWARDMAIL", KEY_FORWARDMAIL},
    {"SAVE", KEY_SAVE},
    {"DOCUMENTS", KEY_DOCUMENTS},
    {"BATTERY", KEY_BATTERY},
    {"BLUETOOTH", KEY_BLUETOOTH},
    {"WLAN", KEY_WLAN},
    {"UWB", KEY_UWB},
    {"UNKNOWN", KEY_UNKNOWN},
    {"VIDEO_NEXT", KEY_VIDEO_NEXT},
    {"VIDEO_PREV", KEY_VIDEO_PREV},
    {"BRIGHTNESS_CYCLE", KEY_BRIGHTNESS_CYCLE},
    {"BRIGHTNESS_AUTO", KEY_BRIGHTNESS_AUTO},
    {"DISPLAY_OFF", KEY_DISPLAY_OFF},
    {"WWAN", KEY_WWAN},
    {"RFKILL", KEY_RFKILL},
    {"MICMUTE", KEY_MICMUTE},
    {"OK", KEY_OK},
    {"SELECT", KEY_SELECT},
    {"GOTO", KEY_GOTO},
    {"CLEAR", KEY_CLEAR},
    {"POWER2", KEY_POWER2},
    {"OPTION", KEY_OPTION},
    {"INFO", KEY_INFO},
    {"TIME", KEY_TIME},
    {"VENDOR", KEY_VENDOR},
    {"ARCHIVE", KEY_ARCHIVE},
    {"PROGRAM", KEY_PROGRAM},
    {"CHANNEL", KEY_CHANNEL},
    {"FAVORITES", KEY_FAVORITES},
    {"EPG", KEY_EPG},
    {"PVR", KEY_PVR},
    {"MHP", KEY_MHP},
    {"LANGUAGE", KEY_LANGUAGE},
    {"TITLE", KEY_TITLE},
    {"SUBTITLE", KEY_SUBTITLE},
    {"ANGLE", KEY_ANGLE},
    {"FULL_SCREEN", KEY_FULL_SCREEN},
    {"MODE", KEY_MODE},
    {"KEYBOARD", KEY_KEYBOARD},
    {"ASPECT_RATIO", KEY_ASPECT_RATIO},
    {"PC", KEY_PC},
    {"TV", KEY_TV},
    {"TV2", KEY_TV2},
    {"VCR", KEY_VCR},
    {"VCR2", KEY_VCR2},
    {"SAT", KEY_SAT},
    {"SAT2", KEY_SAT2},
    {"CD", KEY_CD},
    {"TAPE", KEY_TAPE},
    {"RADIO", KEY_RADIO},
    {"TUNER", KEY_TUNER},
    {"PLAYER", KEY_PLAYER},
    {"TEXT", KEY_TEXT},
    {"DVD", KEY_DVD},
    {"AUX", KEY_AUX},
    {"MP3", KEY_MP3},
    {"AUDIO", KEY_AUDIO},
    {"VIDEO", KEY_VIDEO},
    {"DIRECTORY", KEY_DIRECTORY},
    {"LIST", KEY_LIST},
    {"MEMO", KEY_MEMO},
    {"CALENDAR", KEY_CALENDAR},
    {"RED", KEY_RED},
    {"GREEN", KEY_GREEN},
    {"YELLOW", KEY_YELLOW},
    {"BLUE", KEY_BLUE},
    {"CHANNELUP", KEY_CHANNELUP},
    {"CHANNELDOWN", KEY_CHANNELDOWN},
    {"FIRST", KEY_FIRST},
    {"LAST", KEY_LAST},
    {"AB", KEY_AB},
    {"NEXT", KEY_NEXT},
    {"RESTART", KEY_RESTART},
    {"SLOW", KEY_SLOW},
    {"SHUFFLE", KEY_SHUFFLE},
    {"BREAK", KEY_BREAK},
    {"PREVIOUS", KEY_PREVIOUS},
    {"DIGITS", KEY_DIGITS},
    {"TEEN", KEY_TEEN},
    {"TWEN", KEY_TWEN},
    {"VIDEOPHONE", KEY_VIDEOPHONE},
    {"GAMES", KEY_GAMES},
    {"ZOOMIN", KEY_ZOOMIN},
    {"ZOOMOUT", KEY_ZOOMOUT},
    {"ZOOMRESET", KEY_ZOOMRESET},
    {"WORDPROCESSOR", KEY_WORDPROCESSOR},
    {"EDITOR", KEY_EDITOR},
    {"SPREADSHEET", KEY_SPREADSHEET},
    {"GRAPHICSEDITOR", KEY_GRAPHICSEDITOR},
    {"PRESENTATION", KEY_PRESENTATION},
    {"DATABASE", KEY_DATABASE},
    {"NEWS", KEY_NEWS},
    {"VOICEMAIL", KEY_VOICEMAIL},
    {"ADDRESSBOOK", KEY_ADDRESSBOOK},
    {"MESSENGER", KEY_MESSENGER},
    {"DISPLAYTOGGLE", KEY_DISPLAYTOGGLE},
    {"SPELLCHECK", KEY_SPELLCHECK},
    {"LOGOFF", KEY_LOGOFF},
    {"DOLLAR", KEY_DOLLAR},
    {"EURO", KEY_EURO},
    {"FRAMEBACK", KEY_FRAMEBACK},
    {"FRAMEFORWARD", KEY_FRAMEFORWARD},
    {"CONTEXT_MENU", KEY_CONTEXT_MENU},
    {"MEDIA_REPEAT", KEY_MEDIA_REPEAT},
    {"10CHANNELSUP", KEY_10CHANNELSUP},
    {"10CHANNELSDOWN", KEY_10CHANNELSDOWN},
    {"IMAGES", KEY_IMAGES},
    {"NOTIFICATION_CENTER", KEY_NOTIFICATION_CENTER},
    {"PICKUP_PHONE", KEY_PICKUP_PHONE},
    {"HANGUP_PHONE", KEY_HANGUP_PHONE},
    {"DEL_EOL", KEY_DEL_EOL},
    {"DEL_EOS", KEY_DEL_EOS},
    {"INS_LINE", KEY_INS_LINE},
    {"DEL_LINE", KEY_DEL_LINE},
    {"FN", KEY_FN},
    {"FN_ESC", KEY_FN_ESC},
    {"FN_F1", KEY_FN_F1},
    {"FN_F2", KEY_FN_F2},
    {"FN_F3", KEY_FN_F3},
    {"FN_F4", KEY_FN_F4},
    {"FN_F5", KEY_FN_F5},
    {"FN_F6", KEY_FN_F6},
    {"FN_F7", KEY_FN_F7},
    {"FN_F8", KEY_FN_F8},
    {"FN_F9", KEY_FN_F9},
    {"FN_F10", KEY_FN_F10},
    {"FN_F11", KEY_FN_F11},
    {"FN_F12", KEY_FN_F12},
    {"FN_1", KEY_FN_1},
    {"FN_2", KEY_FN_2},
    {"FN_D", KEY_FN_D},
    {"FN_E", KEY_FN_E},
    {"FN_F", KEY_FN_F},
    {"FN_S", KEY_FN_S},
    {"FN_B", KEY_FN_B},
    {"FN_RIGHT_SHIFT", KEY_FN_RIGHT_SHIFT},
    {"BRL_DOT1", KEY_BRL_DOT1},
    {"BRL_DOT2", KEY_BRL_DOT2},
    {"BRL_DOT3", KEY_BRL_DOT3},
    {"BRL_DOT4", KEY_BRL_DOT4},
    {"BRL_DOT5", KEY_BRL_DOT5},
    {"BRL_DOT6", KEY_BRL_DOT6},
    {"BRL_DOT7", KEY_BRL_DOT7},
    {"BRL_DOT8", KEY_BRL_DOT8},
    {"BRL_DOT9", KEY_BRL_DOT9},
    {"BRL_DOT10", KEY_BRL_DOT10},
    {"NUMERIC_0", KEY_NUMERIC_0},
    {"NUMERIC_1", KEY_NUMERIC_1},
    {"NUMERIC_2", KEY_NUMERIC_2},
    {"NUMERIC_3", KEY_NUMERIC_3},
    {"NUMERIC_4", KEY_NUMERIC_4},
    {"NUMERIC_5", KEY_NUMERIC_5},
    {"NUMERIC_6", KEY_NUMERIC_6},
    {"NUMERIC_7", KEY_NUMERIC_7},
    {"NUMERIC_8", KEY_NUMERIC_8},
    {"NUMERIC_9", KEY_NUMERIC_9},
    {"NUMERIC_STAR", KEY_NUMERIC_STAR},
    {"NUMERIC_POUND", KEY_NUMERIC_POUND},
    {"NUMERIC_A", KEY_NUMERIC_A},
    {"NUMERIC_B", KEY_NUMERIC_B},
    {"NUMERIC_C", KEY_NUMERIC_C},
    {"NUMERIC_D", KEY_NUMERIC_D},
    {"CAMERA_FOCUS", KEY_CAMERA_FOCUS},
    {"WPS_BUTTON", KEY_WPS_BUTTON},
    {"TOUCHPAD_TOGGLE", KEY_TOUCHPAD_TOGGLE},
    {"TOUCHPAD_ON", KEY_TOUCHPAD_ON},
    {"TOUCHPAD_OFF", KEY_TOUCHPAD_OFF},
    {"CAMERA_ZOOMIN", KEY_CAMERA_ZOOMIN},
    {"CAMERA_ZOOMOUT", KEY_CAMERA_ZOOMOUT},
    {"CAMERA_UP", KEY_CAMERA_UP},
    {"CAMERA_DOWN", KEY_CAMERA_DOWN},
    {"CAMERA_LEFT", KEY_CAMERA_LEFT},
    {"CAMERA_RIGHT", KEY_CAMERA_RIGHT},
    {"ATTENDANT_ON", KEY_ATTENDANT_ON},
    {"ATTENDANT_OFF", KEY_ATTENDANT_OFF},
    {"ATTENDANT_TOGGLE", KEY_ATTENDANT_TOGGLE},
    {"LIGHTS_TOGGLE", KEY_LIGHTS_TOGGLE},
    {"ALS_TOGGLE", KEY_ALS_TOGGLE},
    {"ROTATE_LOCK_TOGGLE", KEY_ROTATE_LOCK_TOGGLE},
    {"REFRESH_RATE_TOGGLE", KEY_REFRESH_RATE_TOGGLE},
    {"BUTTONCONFIG", KEY_BUTTONCONFIG},
    {"TASKMANAGER", KEY_TASKMANAGER},
    {"JOURNAL", KEY_JOURNAL},
    {"CONTROLPANEL", KEY_CONTROLPANEL},
    {"APPSELECT", KEY_APPSELECT},
    {"SCREENSAVER", KEY_SCREENSAVER},
    {"VOICECOMMAND", KEY_VOICECOMMAND},
    {"ASSISTANT", KEY_ASSISTANT},
    {"KBD_LAYOUT_NEXT", KEY_KBD_LAYOUT_NEXT},
    {"EMOJI_PICKER", KEY_EMOJI_PICKER},
    {"DICTATE", KEY_DICTATE},
    {"CAMERA_ACCESS_ENABLE", KEY_CAMERA_ACCESS_ENABLE},
    {"CAMERA_ACCESS_DISABLE", KEY_CAMERA_ACCESS_DISABLE},
    {"CAMERA_ACCESS_TOGGLE", KEY_CAMERA_ACCESS_TOGGLE},
    {"BRIGHTNESS_MIN", KEY_BRIGHTNESS_MIN},
    {"BRIGHTNESS_MAX", KEY_BRIGHTNESS_MAX},
    {"KBDINPUTASSIST_PREV", KEY_KBDINPUTASSIST_PREV},
    {"KBDINPUTASSIST_NEXT", KEY_KBDINPUTASSIST_NEXT},
    {"KBDINPUTASSIST_PREVGROUP", KEY_KBDINPUTASSIST_PREVGROUP},
    {"KBDINPUTASSIST_NEXTGROUP", KEY_KBDINPUTASSIST_NEXTGROUP},
    {"KBDINPUTASSIST_ACCEPT", KEY_KBDINPUTASSIST_ACCEPT},
    {"KBDINPUTASSIST_CANCEL", KEY_KBDINPUTASSIST_CANCEL},
    {"RIGHT_UP", KEY_RIGHT_UP},
    {"RIGHT_DOWN", KEY_RIGHT_DOWN},
    {"LEFT_UP", KEY_LEFT_UP},
    {"LEFT_DOWN", KEY_LEFT_DOWN},
    {"ROOT_MENU", KEY_ROOT_MENU},
    {"MEDIA_TOP_MENU", KEY_MEDIA_TOP_MENU},
    {"NUMERIC_11", KEY_NUMERIC_11},
    {"NUMERIC_12", KEY_NUMERIC_12},
    {"AUDIO_DESC", KEY_AUDIO_DESC},
    {"3D_MODE", KEY_3D_MODE},
    {"NEXT_FAVORITE", KEY_NEXT_FAVORITE},
    {"STOP_RECORD", KEY_STOP_RECORD},
    {"PAUSE_RECORD", KEY_PAUSE_RECORD},
    {"VOD", KEY_VOD},
    {"UNMUTE", KEY_UNMUTE},
    {"FASTREVERSE", KEY_FASTREVERSE},
    {"SLOWREVERSE", KEY_SLOWREVERSE},
    {"DATA", KEY_DATA},
    {"ONSCREEN_KEYBOARD", KEY_ONSCREEN_KEYBOARD},
    {"PRIVACY_SCREEN_TOGGLE", KEY_PRIVACY_SCREEN_TOGGLE},
    {"SELECTIVE_SCREENSHOT", KEY_SELECTIVE_SCREENSHOT},
    {"NEXT_ELEMENT", KEY_NEXT_ELEMENT},
    {"PREVIOUS_ELEMENT", KEY_PREVIOUS_ELEMENT},
    {"AUTOPILOT_ENGAGE_TOGGLE", KEY_AUTOPILOT_ENGAGE_TOGGLE},
    {"MARK_WAYPOINT", KEY_MARK_WAYPOINT},
    {"SOS", KEY_SOS},
    {"NAV_CHART", KEY_NAV_CHART},
    {"FISHING_CHART", KEY_FISHING_CHART},
    {"SINGLE_RANGE_RADAR", KEY_SINGLE_RANGE_RADAR},
    {"DUAL_RANGE_RADAR", KEY_DUAL_RANGE_RADAR},
    {"RADAR_OVERLAY", KEY_RADAR_OVERLAY},
    {"TRADITIONAL_SONAR", KEY_TRADITIONAL_SONAR},
    {"CLEARVU_SONAR", KEY_CLEARVU_SONAR},
    {"SIDEVU_SONAR", KEY_SIDEVU_SONAR},
    {"NAV_INFO", KEY_NAV_INFO},
    {"BRIGHTNESS_MENU", KEY_BRIGHTNESS_MENU},
    {"MACRO1", KEY_MACRO1},
    {"MACRO2", KEY_MACRO2},
    {"MACRO3", KEY_MACRO3},
    {"MACRO4", KEY_MACRO4},
    {"MACRO5", KEY_MACRO5},
    {"MACRO6", KEY_MACRO6},
    {"MACRO7", KEY_MACRO7},
    {"MACRO8", KEY_MACRO8},
    {"MACRO9", KEY_MACRO9},
    {"MACRO10", KEY_MACRO10},
    {"MACRO11", KEY_MACRO11},
    {"MACRO12", KEY_MACRO12},
    {"MACRO13", KEY_MACRO13},
    {"MACRO14", KEY_MACRO14},
    {"MACRO15", KEY_MACRO15},
    {"MACRO16", KEY_MACRO16},
    {"MACRO17", KEY_MACRO17},
    {"MACRO18", KEY_MACRO18},
    {"MACRO19", KEY_MACRO19},
    {"MACRO20", KEY_MACRO20},
    {"MACRO21", KEY_MACRO21},
    {"MACRO22", KEY_MACRO22},
    {"MACRO23", KEY_MACRO23},
    {"MACRO24", KEY_MACRO24},
    {"MACRO25", KEY_MACRO25},
    {"MACRO26", KEY_MACRO26},
    {"MACRO27", KEY_MACRO27},
    {"MACRO28", KEY_MACRO28},
    {"MACRO29", KEY_MACRO29},
    {"MACRO30", KEY_MACRO30},
    {"MACRO_RECORD_START", KEY_MACRO_RECORD_START},
    {"MACRO_RECORD_STOP", KEY_MACRO_RECORD_STOP},
    {"MACRO_PRESET_CYCLE", KEY_MACRO_PRESET_CYCLE},
    {"MACRO_PRESET1", KEY_MACRO_PRESET1},
    {"MACRO_PRESET2", KEY_MACRO_PRESET2},
    {"MACRO_PRESET3", KEY_MACRO_PRESET3},
    {"KBD_LCD_MENU1", KEY_KBD_LCD_MENU1},
    {"KBD_LCD_MENU2", KEY_KBD_LCD_MENU2},
    {"KBD_LCD_MENU3", KEY_KBD_LCD_MENU3},
    {"KBD_LCD_MENU4", KEY_KBD_LCD_MENU4},
    {"KBD_LCD_MENU5", KEY_KBD_LCD_MENU5},
    {"MAX", KEY_MAX},
};

// https://invent.kde.org/plasma/kwin/-/blob/cc4d99ae/src/mousebuttons.cpp#L14
static const std::unordered_map<QString, uint32_t> s_mouse = {
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

// Most of the code below is garbage

using namespace libinputactions;

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

static std::any asAny(const Node &node, const std::type_index &type)
{
    if (type == typeid(bool)) {
        return node.as<bool>();
    } else if (type == typeid(CursorShape)) {
        return node.as<CursorShape>();
    } else if (type == typeid(Qt::KeyboardModifiers)) {
        return asSequence(node).as<Qt::KeyboardModifiers>();
    } else if (type == typeid(qreal)) {
        return node.as<qreal>();
    } else if (type == typeid(QPointF)) {
        return node.as<QPointF>();
    } else if (type == typeid(QString)) {
        return node.as<QString>();
    }
    throw Exception(node.Mark(), "Unexpected type");
}

static bool isEnum(const std::type_index &type)
{
    static const std::unordered_set<std::type_index> enums{typeid(Qt::KeyboardModifiers)};
    return enums.contains(type);
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

template<>
struct convert<std::shared_ptr<VariableCondition>>
{
    static bool decode(const Node &node, std::shared_ptr<VariableCondition> &condition)
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
        const auto variable = g_variableManager->getVariable(variableName);
        if (!variable) {
            throw Exception(node.Mark(), QString("Variable '%1' does not exist.").arg(variableName).toStdString());
        }

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
        const auto comparisonOperator = operators.at(operatorRaw);

        const auto rightRaw = raw.mid(secondSpace + 1);
        const auto rightNode = YAML::Load(rightRaw.toStdString());
        std::vector<std::any> right;

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
        condition = std::make_shared<VariableCondition>(variableName, right, comparisonOperator);
        condition->setNegate(negate);
        return true;
    }
};

template<>
struct convert<std::shared_ptr<Condition>>
{
    static bool isLegacy(const Node &node)
    {
        return node.IsMap() && (node["negate"] || node["window_class"] || node["window_state"]);
    }

    static bool decode(const Node &node, std::shared_ptr<Condition> &condition)
    {
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
                    group->add(child.as<std::shared_ptr<Condition>>());
                }
                condition = group;
                return true;
            }

            if (isLegacy(node)) {
                auto group = std::make_shared<ConditionGroup>();
                const auto negate = node["negate"].as<QStringList>(QStringList());
                if (const auto &windowClassNode = node["window_class"]) {
                    const auto value = windowClassNode.as<QString>();
                    auto classGroup = std::make_shared<ConditionGroup>(ConditionGroupMode::Any);
                    classGroup->add(std::make_shared<VariableCondition>("window_class", value, ComparisonOperator::Regex));
                    classGroup->add(std::make_shared<VariableCondition>("window_name", value, ComparisonOperator::Regex));
                    classGroup->setNegate(negate.contains("window_class"));
                    group->add(classGroup);
                }
                if (const auto &windowStateNode = node["window_state"]) {
                    const auto value = windowStateNode.as<QStringList>(QStringList());
                    auto classGroup = std::make_shared<ConditionGroup>(ConditionGroupMode::Any);
                    if (value.contains("fullscreen")) {
                        classGroup->add(std::make_shared<VariableCondition>("window_fullscreen", true, ComparisonOperator::EqualTo));
                    }
                    if (value.contains("maximized")) {
                        classGroup->add(std::make_shared<VariableCondition>("window_maximized", true, ComparisonOperator::EqualTo));
                    }
                    classGroup->setNegate(negate.contains("window_state"));
                    group->add(classGroup);
                }
                condition = group;
                return true;
            }
        }

        // Not in any group
        if (node.IsSequence()) {
            auto group = std::make_shared<ConditionGroup>(isLegacy(node[0]) ? ConditionGroupMode::Any : ConditionGroupMode::All);
            for (const auto &child : node) {
                group->add(child.as<std::shared_ptr<Condition>>());
            }
            condition = group;
            return true;
        }

        if (node.IsScalar()) {
            // Hack to load negated conditions without forcing users to quote the entire thing
            auto conditionNode = node;
            const auto tag = node.Tag();
            if (tag != "!" && tag.starts_with('!')) {
                conditionNode = node.Tag() + " " + node.as<std::string>();
            }

            const auto raw = conditionNode.as<QString>("");
            if (raw.startsWith("$") || raw.startsWith("!$")) {
                condition = node.as<std::shared_ptr<VariableCondition>>();
                return true;
            }
        }
        return false;
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
struct convert<std::unique_ptr<InputEventHandler>>
{
    static bool decode(const Node &node, std::unique_ptr<InputEventHandler> &handler)
    {
        handler = std::make_unique<InputEventHandler>();
        if (const auto &blacklistNode = node["blacklist"]) {
            handler->setDeviceNameBlacklist(blacklistNode.as<std::set<QString>>());
        } else if (const auto &whitelistNode = node["whitelist"]) {
            handler->setDeviceNameWhitelist(whitelistNode.as<std::set<QString>>());
        }
        return true;
    }
};

template<>
struct convert<std::vector<std::unique_ptr<InputEventHandler>>>
{
    template<typename T>
    static std::unique_ptr<InputEventHandler> decodeHandler(const Node &node)
    {
        auto result = node.as<std::unique_ptr<InputEventHandler>>();
        result->setTriggerHandler(node.as<std::unique_ptr<T>>());
        return result;
    }

    static bool decode(const Node &node, std::vector<std::unique_ptr<InputEventHandler>> &handlers)
    {
        if (const auto &mouseNode = node["keyboard"]) {
            for (const auto &keyboardHandler : asSequence(mouseNode)) {
                handlers.push_back(decodeHandler<KeyboardTriggerHandler>(keyboardHandler));
            }
        }
        if (const auto &mouseNode = node["mouse"]) {
            for (const auto &mouseHandler : asSequence(mouseNode)) {
                handlers.push_back(decodeHandler<MouseTriggerHandler>(mouseHandler));
            }
        }
        if (const auto &touchpadNode = node["touchpad"]) {
            for (const auto &touchpadHandler : asSequence(touchpadNode)) {
                handlers.push_back(decodeHandler<TouchpadTriggerHandler>(touchpadHandler));
            }
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

template<>
struct convert<std::unique_ptr<Trigger>>
{
    static bool decode(const Node &node, std::unique_ptr<Trigger> &trigger)
    {
        const auto type = node["type"].as<QString>();
        if (type == "click") {
            trigger = std::make_unique<Trigger>(TriggerType::Click);
        } else if (type == "hold" || type == "press") {
            auto pressTrigger = new PressTrigger;
            pressTrigger->m_instant = node["instant"].as<bool>(false);
            trigger.reset(pressTrigger);
        } else if (type == "pinch") {
            trigger = std::make_unique<DirectionalMotionTrigger>(TriggerType::Pinch, static_cast<TriggerDirection>(node["direction"].as<PinchDirection>()));
        } else if (type == "shortcut") {
            trigger = std::make_unique<KeyboardShortcutTrigger>(node["shortcut"].as<KeyboardShortcut>());
        } else if (type == "stroke") {
            trigger = std::make_unique<StrokeTrigger>(asSequence(node["strokes"]).as<std::vector<Stroke>>());
        } else if (type == "swipe") {
            trigger = std::make_unique<DirectionalMotionTrigger>(TriggerType::Swipe, static_cast<TriggerDirection>(node["direction"].as<SwipeDirection>()));
        } else if (type == "rotate") {
            trigger = std::make_unique<DirectionalMotionTrigger>(TriggerType::Rotate, static_cast<TriggerDirection>(node["direction"].as<RotateDirection>()));
        } else if (type == "wheel") {
            trigger = std::make_unique<WheelTrigger>(static_cast<TriggerDirection>(node["direction"].as<SwipeDirection>()));
        } else {
            throw Exception(node.Mark(), "Invalid trigger type");
        }

        auto conditionGroup = std::make_shared<ConditionGroup>();

        if (const auto &idNode = node["id"]) {
            trigger->setId(idNode.as<QString>());
        }
        if (const auto &fingersNode = node["fingers"]) {
            auto range = fingersNode.as<Range<qreal>>();
            if (!range.max()) {
                conditionGroup->add(std::make_shared<VariableCondition>(BuiltinVariables::Fingers, range.min().value(), ComparisonOperator::EqualTo));
            } else {
                conditionGroup->add(std::make_shared<VariableCondition>(BuiltinVariables::Fingers,
                                                                        std::vector<std::any>{range.min().value(), range.max().value()},
                                                                        ComparisonOperator::Between));
            }
        }
        if (const auto &thresholdNode = node["threshold"]) {
            trigger->setThreshold(thresholdNode.as<Range<qreal>>());
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
                conditionGroup->add(std::make_shared<VariableCondition>(BuiltinVariables::KeyboardModifiers, modifiers.value(), ComparisonOperator::EqualTo));
            }
        }
        if (const auto &mouseButtonsNode = node["mouse_buttons"]) {
            trigger->setMouseButtons(mouseButtonsNode.as<std::vector<Qt::MouseButton>>());
        }
        if (const auto &mouseButtonsExactOrderNode = node["mouse_buttons_exact_order"]) {
            trigger->setMouseButtonsExactOrder(mouseButtonsExactOrderNode.as<bool>());
        }
        if (const auto &conditionsNode = node["conditions"]) {
            conditionGroup->add(conditionsNode.as<std::shared_ptr<Condition>>());
        }
        if (const auto &endConditionsNode = node["end_conditions"]) {
            trigger->setEndCondition(endConditionsNode.as<std::shared_ptr<Condition>>());
        }
        for (const auto &actionNode : node["actions"]) {
            trigger->addAction(actionNode.as<std::unique_ptr<TriggerAction>>());
        }
        if (const auto &clearModifiersNode = node["clear_modifiers"]) {
            trigger->setClearModifiers(clearModifiersNode.as<bool>());
        }
        if (const auto &setLastTriggerNode = node["set_last_trigger"]) {
            trigger->setSetLastTrigger(setLastTriggerNode.as<bool>());
        }

        if (auto *motionTrigger = dynamic_cast<MotionTrigger *>(trigger.get())) {
            if (const auto &speedNode = node["speed"]) {
                motionTrigger->setSpeed(speedNode.as<TriggerSpeed>());
            }
        }

        trigger->setActivationCondition(conditionGroup);
        return true;
    }
};

template<>
struct convert<std::unique_ptr<TriggerAction>>
{
    static bool decode(const Node &node, std::unique_ptr<TriggerAction> &value)
    {
        value = std::make_unique<TriggerAction>(node.as<std::shared_ptr<Action>>());

        if (const auto &thresholdNode = node["threshold"]) {
            value->m_threshold = thresholdNode.as<Range<qreal>>();
        }

        value->m_on = node["on"].as<On>(On::End);
        if (value->m_on == On::Begin && value->m_threshold && (value->m_threshold->min() || value->m_threshold->max())) {
            throw Exception(node.Mark(), "Begin actions can't have thresholds");
        }
        value->m_interval = node["interval"].as<ActionInterval>(ActionInterval());

        return true;
    }
};

template<>
struct convert<std::shared_ptr<Action>>
{
    static bool decode(const Node &node, std::shared_ptr<Action> &value)
    {
        if (const auto &commandNode = node["command"]) {
            auto action = std::make_shared<CommandAction>(commandNode.as<libinputactions::Value<QString>>());
            if (const auto &waitNode = node["wait"]) {
                action->m_wait = waitNode.as<bool>();
            }
            value = action;
        } else if (const auto &inputNode = node["input"]) {
            auto action = std::make_shared<InputAction>(inputNode.as<std::vector<InputAction::Item>>());
            if (const auto &delayNode = node["delay"]) {
                action->m_delay = delayNode.as<std::chrono::milliseconds>();
            }
            value = action;
        } else if (const auto &plasmaShortcutNode = node["plasma_shortcut"]) {
            const auto split = plasmaShortcutNode.as<QString>().split(",");
            if (split.length() != 2) {
                throw Exception(node.Mark(), "Invalid Plasma shortcut format");
            }
            value = std::make_shared<PlasmaGlobalShortcutAction>(split[0], split[1]);
        } else if (const auto &oneNode = node["one"]) {
            value = std::make_shared<ActionGroup>(oneNode.as<std::vector<std::shared_ptr<Action>>>(), ActionGroup::ExecutionMode::First);
        } else {
            throw Exception(node.Mark(), "Action has no valid action property");
        }

        if (const auto &conditionsNode = node["conditions"]) {
            value->m_condition = conditionsNode.as<std::shared_ptr<Condition>>();
        }
        if (const auto &idNode = node["id"]) {
            value->m_id = idNode.as<QString>();
        }

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
        if (const auto &eventsNode = speedNode["events"]) {
            motionHandler->setSpeedInputEventsToSample(eventsNode.as<uint8_t>());
        }
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

        if (const auto &motionTimeoutNode = node["motion_timeout"]) {
            mouseTriggerHandler->setMotionTimeout(motionTimeoutNode.as<uint32_t>());
        }
        if (const auto &pressTimeoutNode = node["press_timeout"]) {
            mouseTriggerHandler->setPressTimeout(pressTimeoutNode.as<uint32_t>());
        }
        if (const auto &unblockButtonsOnTimeout = node["unblock_buttons_on_timeout"]) {
            mouseTriggerHandler->setUnblockButtonsOnTimeout(unblockButtonsOnTimeout.as<bool>());
        }

        return true;
    }
};

template<>
struct convert<std::unique_ptr<TouchpadTriggerHandler>>
{
    static bool decode(const Node &node, std::unique_ptr<TouchpadTriggerHandler> &handler)
    {
        auto *touchpadTriggerHandler = new TouchpadTriggerHandler;
        handler.reset(touchpadTriggerHandler);
        decodeMultiTouchMotionTriggerHandler(node, handler.get());

        if (const auto &deltaMultiplierNode = node["delta_multiplier"]) {
            touchpadTriggerHandler->setSwipeDeltaMultiplier(deltaMultiplierNode.as<qreal>());
        }
        if (const auto &clickTimeoutNode = node["click_timeout"]) {
            touchpadTriggerHandler->setClickTimeout(clickTimeoutNode.as<uint32_t>());
        }

        return true;
    }
};

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
                 {"update", On::Update},
                 {"cancel", On::Cancel},
                 {"end", On::End},
                 {"end_cancel", On::EndCancel},
             }))
ENUM_DECODER(CursorShape, "cursor shape", CURSOR_SHAPES)
ENUM_DECODER(Qt::MouseButton, "mouse button",
             (std::unordered_map<QString, Qt::MouseButton>{
                 {"left", Qt::MouseButton::LeftButton},
                 {"middle", Qt::MouseButton::MiddleButton},
                 {"right", Qt::MouseButton::RightButton},
                 {"back", Qt::MouseButton::ExtraButton1},
                 {"forward", Qt::MouseButton::ExtraButton2},
                 {"extra1", Qt::MouseButton::ExtraButton1},
                 {"extra2", Qt::MouseButton::ExtraButton2},
                 {"extra3", Qt::MouseButton::ExtraButton3},
                 {"extra4", Qt::MouseButton::ExtraButton4},
                 {"extra5", Qt::MouseButton::ExtraButton5},
                 {"extra6", Qt::MouseButton::ExtraButton6},
                 {"extra7", Qt::MouseButton::ExtraButton7},
                 {"extra8", Qt::MouseButton::ExtraButton8},
                 {"extra9", Qt::MouseButton::ExtraButton9},
                 {"extra10", Qt::MouseButton::ExtraButton10},
                 {"extra11", Qt::MouseButton::ExtraButton11},
                 {"extra12", Qt::MouseButton::ExtraButton12},
                 {"extra13", Qt::MouseButton::ExtraButton13},
             }))
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
                            .keyboardText = actionNode["text"].as<libinputactions::Value<QString>>(),
                        });
                    } else {
                        const auto actionRaw = actionNode.as<QString>().toUpper();
                        if (actionRaw.startsWith("+") || actionRaw.startsWith("-")) {
                            const auto key = actionRaw.mid(1);
                            if (!s_keyboard.contains(key)) {
                                throw Exception(node.Mark(), ("Invalid keyboard key ('" + key + "')").toStdString());
                            }

                            if (actionRaw[0] == '+') {
                                value.push_back({
                                    .keyboardPress = s_keyboard.at(key),
                                });
                            } else {
                                value.push_back({
                                    .keyboardRelease = s_keyboard.at(key),
                                });
                            }
                        } else {
                            std::vector<uint32_t> keys;
                            for (const auto &keyRaw : actionRaw.split("+")) {
                                if (!s_keyboard.contains(keyRaw)) {
                                    throw Exception(node.Mark(), ("Invalid keyboard key ('" + keyRaw + "')").toStdString());
                                }
                                keys.push_back(s_keyboard.at(keyRaw));
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
                    actionRaw = actionRaw.toUpper();
                    if (actionRaw.startsWith("+") || actionRaw.startsWith("-")) {
                        const auto button = actionRaw.mid(1);
                        if (!s_mouse.contains(button)) {
                            throw Exception(node.Mark(), ("Invalid mouse button ('" + button + "')").toStdString());
                        }

                        if (actionRaw[0] == '+') {
                            value.push_back({
                                .mousePress = s_mouse.at(button),
                            });
                        } else {
                            value.push_back({
                                .mouseRelease = s_mouse.at(button),
                            });
                        }
                    } else if (actionRaw.startsWith("MOVE_BY_DELTA")) {
                        value.push_back({
                            .mouseMoveRelativeByDelta = true,
                        });
                    } else if (actionRaw.startsWith("MOVE_BY")) {
                        const auto split = actionRaw.split(" ");
                        value.push_back({
                            .mouseMoveRelative = QPointF(split[1].toFloat(), split[2].toFloat()),
                        });
                    } else if (actionRaw.startsWith("MOVE_TO")) {
                        const auto split = actionRaw.split(" ");
                        value.push_back({
                            .mouseMoveAbsolute = QPointF(split[1].toFloat(), split[2].toFloat()),
                        });
                    } else {
                        std::vector<uint32_t> buttons;
                        for (const auto &buttonRaw : actionRaw.split("+")) {
                            if (!s_mouse.contains(buttonRaw)) {
                                throw Exception(node.Mark(), ("Invalid mouse button ('" + buttonRaw + "')").toStdString());
                            }
                            buttons.push_back(s_mouse.at(buttonRaw));
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
struct convert<KeyboardShortcut>
{
    static bool decode(const Node &node, KeyboardShortcut &value)
    {
        for (const auto &keyNode : node) {
            const auto key = keyNode.as<QString>().toUpper();
            if (!s_keyboard.contains(key)) {
                throw Exception(node.Mark(), ("Invalid keyboard key ('" + key + "')").toStdString());
            }
            value.keys.insert(s_keyboard.at(key));
        }
        return true;
    }
};

template<>
struct convert<InputDeviceProperties>
{
    static bool decode(const Node &node, InputDeviceProperties &value)
    {
        if (const auto &multiTouchNode = node["__multiTouch"]) {
            value.setMultiTouch(multiTouchNode.as<bool>());
        }
        if (const auto &buttonPad = node["buttonpad"]) {
            value.setButtonPad(buttonPad.as<bool>());
        }
        if (const auto &pressureRangesNode = node["pressure_ranges"]) {
            if (const auto &thumbNode = pressureRangesNode["thumb"]) {
                value.setThumbPressureRange(thumbNode.as<Range<uint32_t>>());
            }
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
struct convert<libinputactions::Value<T>>
{
    static bool decode(const Node &node, libinputactions::Value<T> &value)
    {
        if (node.IsMap()) {
            if (const auto &commandNode = node["command"]) {
                value = libinputactions::Value<T>::command(commandNode.as<libinputactions::Value<QString>>());
            }
        } else {
            const auto raw = node.as<QString>();
            // TODO Variable reference only
            if (typeid(T) == typeid(QString)) { // String with possible variable references (too lazy to check)
                value = libinputactions::Value<T>(Expression<QString>(raw));
            } else {
                value = libinputactions::Value<T>(node.as<T>());
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
