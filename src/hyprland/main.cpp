#include <hyprland/src/plugins/PluginAPI.hpp>
#undef HANDLE

#include <QCoreApplication>

#include "Plugin.h"

inline std::unique_ptr<Plugin> plugin;

// Required for event loop
static int argc = 0;
inline QCoreApplication app(argc, nullptr);

APICALL EXPORT std::string PLUGIN_API_VERSION()
{
    return HYPRLAND_API_VERSION;
}

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(void *handle)
{
    const std::string hash = __hyprland_api_get_hash();
    if (hash != GIT_COMMIT_HASH) {
        HyprlandAPI::addNotification(handle, "[InputActions] Mismatched headers! Can't proceed.", CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        throw std::runtime_error("[InputActions] Version mismatch");
    }

    plugin = std::make_unique<Plugin>(handle);
    return {"InputActions", "An amazing plugin that is going to change the world!", "Me", "1.0"};
}

APICALL EXPORT void PLUGIN_EXIT()
{
    plugin.reset();
}