/*
    Input Actions - Input handler that executes user-defined actions
    Copyright (C) 2025 Marcin Woźniak

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

#include <QString>
#include <map>

namespace libinputactions
{

// https://wayland.app/protocols/cursor-shape-v1#wp_cursor_shape_device_v1:enum:shape
enum class CursorShape
{
    Default,
    ContextMenu,
    Help,
    Pointer,
    Progress,
    Wait,
    Cell,
    Crosshair,
    Text,
    VerticalText,
    Alias,
    Copy,
    Move,
    NoDrop,
    NotAllowed,
    Grab,
    Grabbing,
    EResize,
    NResize,
    NEResize,
    NWResize,
    SResize,
    SEResize,
    SWResize,
    WResize,
    EWResize,
    NSResize,
    NESWResize,
    NWSEResize,
    ColResize,
    RowResize,
    AllScroll,
    ZoomIn,
    ZoomOut,
    DndAsk,
    AllResize,

    // KWin https://invent.kde.org/plasma/kwin/-/blob/d36646652272d5793eb07498db2d4e45109536fb/src/cursor.cpp#L585
    UpArrow,
};

static const std::map<QString, CursorShape> CURSOR_SHAPES = {
    {"default", CursorShape::Default},
    {"context_menu", CursorShape::ContextMenu},
    {"help", CursorShape::Help},
    {"pointer", CursorShape::Pointer},
    {"progress", CursorShape::Progress},
    {"wait", CursorShape::Wait},
    {"cell", CursorShape::Cell},
    {"crosshair", CursorShape::Crosshair},
    {"text", CursorShape::Text},
    {"vertical_text", CursorShape::VerticalText},
    {"alias", CursorShape::Alias},
    {"copy", CursorShape::Copy},
    {"move", CursorShape::Move},
    {"no_drop", CursorShape::NoDrop},
    {"not_allowed", CursorShape::NotAllowed},
    {"grab", CursorShape::Grab},
    {"grabbing", CursorShape::Grabbing},
    {"e_resize", CursorShape::EResize},
    {"n_resize", CursorShape::NResize},
    {"ne_resize", CursorShape::NEResize},
    {"nw_resize", CursorShape::NWResize},
    {"s_resize", CursorShape::SResize},
    {"se_resize", CursorShape::SEResize},
    {"sw_resize", CursorShape::SWResize},
    {"w_resize", CursorShape::WResize},
    {"ew_resize", CursorShape::EWResize},
    {"ns_resize", CursorShape::NSResize},
    {"nesw_resize", CursorShape::NESWResize},
    {"nwse_resize", CursorShape::NWSEResize},
    {"col_resize", CursorShape::ColResize},
    {"row_resize", CursorShape::RowResize},
    {"all_scroll", CursorShape::AllScroll},
    {"zoom_in", CursorShape::ZoomIn},
    {"zoom_out", CursorShape::ZoomOut},
    {"dnd_ask", CursorShape::DndAsk},
    {"all_resize", CursorShape::AllResize},
    {"up_arrow", CursorShape::UpArrow},

    // Aliases
    {"left_ptr", CursorShape::Default},
};

class CursorShapeProvider
{
public:
    CursorShapeProvider() = default;
    virtual ~CursorShapeProvider() = default;

    /**
     * @returns The current cursor shape, or std::nullopt if not available.
     */
    virtual std::optional<CursorShape> cursorShape()
    {
        return {};
    };
};

inline std::shared_ptr<CursorShapeProvider> g_cursorShapeProvider;

}