#include "settings.h"
#include "source.h"
#include <util/util.hpp>
#include <util/platform.h>

void Settings::setDefaultSettings(obs_data_t* settings)
{
    obs_data_set_default_bool(settings, "enabled", true);
    obs_data_set_default_int(settings, "timeout", 60);
}

Settings::Settings()
{
    os_mkdirs(obs_current_module());
}

OBSData Settings::getSettings()
{
    std::scoped_lock lock(mtx);

    if (cacheDirty)
    {
        BPtr<char> settingsFile = SETTINGS_PATH();

        BPtr<char> jsonData = os_quick_read_utf8_file(settingsFile);
        auto data = !!jsonData ? obs_data_create_from_json(jsonData) : obs_data_create();
        
        OBSData settings(data);
        setDefaultSettings(settings);
        obs_data_release(data);

        cachedSettings = settings;
        cacheDirty = false;
        return settings;
    }

    return cachedSettings;
}

void Settings::saveSettings()
{
    std::scoped_lock lock(mtx);

    BPtr<char> settingsFile = SETTINGS_PATH();
    obs_data_save_json_safe(cachedSettings, settingsFile, "tmp", "bak");
    
    cacheDirty = true;
}

OBSDataArray Settings::getArray(obs_data_t* settings, const char* key)
{
    auto arr = obs_data_get_array(settings, key);
    if (obs_data_array_count(arr) == 0)
        arr = obs_data_array_create();
    
    auto out = OBSDataArray(arr);
    obs_data_array_release(arr);
    return out;
}
