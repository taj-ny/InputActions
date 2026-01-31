let windowUnderPointer;

const dataAccessors = {
    active_window_class: () => workspace.activeWindow ? workspace.activeWindow.resourceClass : null,
    active_window_fullscreen: () => workspace.activeWindow ? workspace.activeWindow.fullScreen : null,
    active_window_id: () => workspace.activeWindow ? workspace.activeWindow.internalId : null,
    active_window_maximized: () => workspace.activeWindow ? workspace.activeWindow.maximized : false,
    active_window_name: () => workspace.activeWindow ? workspace.activeWindow.resourceName : null,
    active_window_pid: () => workspace.activeWindow ? workspace.activeWindow.pid : null,
    active_window_title: () => workspace.activeWindow ? workspace.activeWindow.caption : null,
    pointer_position_global: () => [workspace.cursorPos.x, workspace.cursorPos.y],
    pointer_position_screen_percentage: () => {
        const output = workspace.screenAt(workspace.cursorPos);
        return [(workspace.cursorPos.x - output.geometry.x) / output.geometry.width, (workspace.cursorPos.y - output.geometry.y) / output.geometry.height];
    },
    window_under_pointer_class: () => windowUnderPointer ? windowUnderPointer.resourceClass : null,
    window_under_pointer_fullscreen: () => windowUnderPointer ? windowUnderPointer.fullScreen : null,
    window_under_pointer_geometry: () => windowUnderPointer ? [windowUnderPointer.x, windowUnderPointer.y, windowUnderPointer.width, windowUnderPointer.height] : null,
    window_under_pointer_id: () => windowUnderPointer ? windowUnderPointer.internalId : null,
    window_under_pointer_maximized: () => windowUnderPointer ? windowUnderPointer.maximized : false,
    window_under_pointer_name: () => windowUnderPointer ? windowUnderPointer.resourceName : null,
    window_under_pointer_pid: () => windowUnderPointer ? windowUnderPointer.pid : null,
    window_under_pointer_title: () => windowUnderPointer ? windowUnderPointer.caption : null
};

let activeWindow;
let pointerPositionChanged;

let pointerPositionTimer = new QTimer();
pointerPositionTimer.interval = 100;
pointerPositionTimer.singleShot = false;
pointerPositionTimer.timeout.connect(() => {
    if (pointerPositionChanged) {
        const previous = windowUnderPointer;
        windowUnderPointer = workspace.windowAt(workspace.cursorPos, 1)[0];
        if (previous && previous != windowUnderPointer) {
            previous.captionChanged.disconnect(onWindowUnderPointerCaptionChanged);
            previous.maximizedAboutToChange.disconnect(onWindowUnderPointerMaximizedAboutToChange);
        }

        let data = [
            "pointer_position_global",
            "pointer_position_screen_percentage",
            "window_under_pointer_geometry"
        ];
        if (previous != windowUnderPointer) {
            data = [
                ...data,
                "window_under_pointer_class",
                "window_under_pointer_fullscreen",
                "window_under_pointer_id",
                "window_under_pointer_name",
                "window_under_pointer_pid",
                "window_under_pointer_title"
            ];
        }
        sendData(data);

        if (windowUnderPointer && previous != windowUnderPointer) {
            windowUnderPointer.captionChanged.connect(onWindowUnderPointerCaptionChanged);
            windowUnderPointer.maximizedAboutToChange.connect(onWindowUnderPointerMaximizedAboutToChange);
        }

        pointerPositionChanged = false;
    }
});
pointerPositionTimer.start();

function sendData(keys) {
    if (!keys.length) {
        keys = Object.keys(dataAccessors);
    }

    let data = {};
    for (let key of keys) {
        data[key] = dataAccessors[key]();
    }

    callDBus("org.inputactions", "/", "org.inputactions", "environmentState", JSON.stringify(data));
}

sendData([]);

function onActiveWindowCaptionChanged() {
    sendData(["active_window_title"]);
}
function onActiveWindowMaximizedAboutToChange(mode) {
    activeWindow.maximized = mode == 3;
    sendData(["active_window_maximized"]);
}
function onWindowUnderPointerMaximizedAboutToChange(mode) {
    windowUnderPointer.maximized = mode == 3;
    sendData(["window_under_pointer_maximized"]);
}
function onWindowUnderPointerCaptionChanged() {
    sendData(["window_under_pointer_title"]);
}

workspace.windowActivated.connect(w => {
    if (activeWindow) {
        activeWindow.captionChanged.disconnect(onActiveWindowCaptionChanged);
        activeWindow.maximizedAboutToChange.disconnect(onActiveWindowMaximizedAboutToChange);
    }
    activeWindow = w;
    sendData([
        "active_window_class",
        "active_window_fullscreen",
        "active_window_id",
        "active_window_maximized",
        "active_window_name",
        "active_window_pid",
        "active_window_title"
    ]);

    if (w) {
        w.captionChanged.connect(onActiveWindowCaptionChanged);
        w.maximizedAboutToChange.connect(onActiveWindowMaximizedAboutToChange);
    }
});
workspace.cursorPosChanged.connect(() => pointerPositionChanged = true);