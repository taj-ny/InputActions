#include <hyprland/src/plugins/PluginAPI.hpp>
#undef HANDLE

#include <QCoreApplication>

#include "Plugin.h"

inline void *PHANDLE = nullptr;
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
    PHANDLE = handle;

    const std::string HASH = __hyprland_api_get_hash();

    // ALWAYS add this to your plugins. It will prevent random crashes coming from
    // mismatched header versions.
    if (HASH != GIT_COMMIT_HASH) {
        HyprlandAPI::addNotification(PHANDLE, "[MyPlugin] Mismatched headers! Can't proceed.",
                                     CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        throw std::runtime_error("[MyPlugin] Version mismatch");
    }

    plugin = std::make_unique<Plugin>(PHANDLE);

    return {"MyPlugin", "An amazing plugin that is going to change the world!", "Me", "1.0"};
}

APICALL EXPORT void PLUGIN_EXIT()
{
    plugin.reset();
}