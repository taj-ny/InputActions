import Clutter from "gi://Clutter";
import Gio from "gi://Gio";
import GLib from "gi://GLib";
import Meta from "gi://Meta";

import {Extension} from "resource:///org/gnome/shell/extensions/extension.js";

const WINDOW_KEYS = [
    "active_window_class",
    "active_window_id",
    "active_window_fullscreen",
    "active_window_maximized",
    "active_window_name",
    "active_window_title",
    "window_under_pointer_class",
    "window_under_pointer_fullscreen",
    "window_under_pointer_id",
    "window_under_pointer_maximized",
    "window_under_pointer_geometry",
    "window_under_pointer_name",
    "window_under_pointer_title"
];

export default class MyExtension extends Extension {
    enable() {
        this._dbusDataRequestedSignalSubscription = Gio.DBus.session.signal_subscribe(
            "org.inputactions",
            "org.inputactions",
            "environmentStateRequested",
            "/",
            null,
            Gio.DBusSignalFlags.NONE,
            (connection, sender, path, iface, signal, params) => {
                this._sendData(params.deep_unpack()[0]);
            }
        );

        this._connections = new Map();
        this._connect(global.display, "notify::focus-window", () => this._onActiveWindowChanged());

        this._cursorTracker = global.backend.get_cursor_tracker();
        this._connect(this._cursorTracker, "position-invalidated", () => this._onPointerMotion());

        this._pointerPositionTimerId = GLib.timeout_add(GLib.PRIORITY_DEFAULT, 100, () => this._onPointerPositionTimerTick());

        this._dataAccessors = {
            active_window_class: () => this._activeWindow ? this._activeWindow.get_wm_class() : null,
            active_window_id: () => this._activeWindow ? this._activeWindow.get_id() : null,
            active_window_maximized: () => {
                return this._activeWindow ? this._activeWindow.maximized_horizontally && this._activeWindow.maximized_vertically : null;
            },
            active_window_fullscreen: () => this._activeWindow ? this._activeWindow.is_fullscreen() : null,
            active_window_name: () => this._activeWindow ? this._activeWindow.get_wm_class_instance() : null,
            active_window_title: () => this._activeWindow ? this._activeWindow.get_title() : null,
            pointer_position_global: () => {
                const pos = this._cursorTracker.get_pointer()[0];
                return [pos.x, pos.y];
            },
            pointer_position_screen_percentage: () => {
                const displayGeometry = global.display.get_monitor_geometry(global.display.get_current_monitor());
                let pointerPosition = this._cursorTracker.get_pointer()[0];
                return [(pointerPosition.x - displayGeometry.x) / displayGeometry.width, (pointerPosition.y - displayGeometry.y) / displayGeometry.height];
            },
            window_under_pointer_id: () => this._windowUnderPointer ? this._windowUnderPointer.get_id() : null,
            window_under_pointer_class: () => this._windowUnderPointer ? this._windowUnderPointer.get_wm_class() : null,
            window_under_pointer_geometry: () => {
                if (this._windowUnderPointer) {
                    const rect = this._windowUnderPointer.get_frame_rect();
                    return [rect.x, rect.y, rect.width, rect.height];
                }
                return null;
            },
            window_under_pointer_maximized: () => {
                return this._windowUnderPointer ? this._windowUnderPointer.maximized_horizontally && this._windowUnderPointer.maximized_vertically : null;
            },
            window_under_pointer_fullscreen: () => this._windowUnderPointer ? this._windowUnderPointer.is_fullscreen() : null,
            window_under_pointer_name: () => this._windowUnderPointer ? this._windowUnderPointer.get_wm_class_instance() : null,
            window_under_pointer_title: () => this._windowUnderPointer ? this._windowUnderPointer.get_title() : null
        };
        this._onActiveWindowChanged();
        this._onPointerMotion();

        this._sendData([]);
    }

    disable() {
        Gio.DBus.session.signal_unsubscribe(this._dbusDataRequestedSignalSubscription);
        for (let [key, value] of this._connections) {
            key.disconnect(value);
        }
        GLib.Source.remove(this._pointerPositionTimerId);
    }

    _connect(obj, signal, callback) {
        if (!this._connections.has(obj)) {
            this._connections.set(obj, []);
        }
        this._connections.get(obj).push(obj.connect(signal, callback));
    }

    _disconnect(obj) {
        if (!this._connections.has(obj)) {
            return;
        }

        for (let connection of this._connections.get(obj)) {
            obj.disconnect(connection);
        }
        this._connections.delete(obj);
    }

    _onActiveWindowChanged() {
        this._activeWindow = global.display.get_focus_window();
        this._sendData(WINDOW_KEYS);
    }

    _onPointerPositionTimerTick() {
        if (!this._pointerPositionChanged) {
            return true;
        }
        this._pointerPositionChanged = false;

        const [x, y, _] = global.get_pointer();
        let actor = global.stage.get_actor_at_pos(Clutter.PickMode.REACTIVE, x, y);
        while (actor && !(actor instanceof Meta.WindowActor)) {
            actor = actor.get_parent();
        }

        const pointerPos = this._cursorTracker.get_pointer()[0];
        let window = actor ? actor.get_meta_window() : null;
        if (window) {
            const windowRect = window.get_frame_rect();
            if (pointerPos.x < windowRect.x || pointerPos.y < windowRect.y || pointerPos.x > (windowRect.x + windowRect.width) || pointerPos.y > (windowRect.y + windowRect.height)) {
                window = null;
            }
        }

        let data = ["pointer_position_global", "pointer_position_screen_percentage", "window_under_pointer_geometry"];

        if (this._windowUnderPointer != window) {
            this._windowUnderPointer = window;
            this._disconnect(this._windowUnderPointer);
            if (window) {
                this._connect(window, "notify::fullscreen", () => this._sendData(["active_window_fullscreen", "window_under_pointer_fullscreen"]));
                this._connect(window, "notify::maximized-horizontally", () => this._sendData(["active_window_maximized", "window_under_pointer_maximized"]));
                this._connect(window, "notify::maximized-vertically", () => this._sendData(["active_window_maximized", "window_under_pointer_maximized"]));
                this._connect(window, "notify::title", () => this._sendData(["active_window_title", "window_under_pointer_title"]));
                this._connect(window, "notify::wm-class", () => this._sendData(["active_window_class", "active_window_name", "window_under_pointer_class", "window_under_pointer_name"]));
            }

            data = [...data, ...WINDOW_KEYS];
        }

        this._sendData(data);
        return true;
    }

    _onPointerMotion() {
        this._pointerPositionChanged = true;
    }

    _sendData(keys) {
        if (!keys.length) {
            keys = Object.keys(this._dataAccessors);
        }

        let data = {};
        for (let key of keys) {
            data[key] = this._dataAccessors[key]();
        }

        Gio.DBus.session.call(
            "org.inputactions",
            "/",
            "org.inputactions",
            "environmentState",
            new GLib.Variant("(s)", [JSON.stringify(data)]),
            null,
            Gio.DBusCallFlags.NONE,
            1,
            null,
            null
        );
    }
}
