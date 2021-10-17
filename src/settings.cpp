#include "settings.h"
#include "source.h"
#include <util/util.hpp>
#include <util/platform.h>

void Settings::setDefaultSettings(obs_data_t* settings)
{
    obs_data_set_default_bool(settings, "enabled", true);
    obs_data_set_default_int(settings, "timeout", 60);
    obs_data_set_default_int(settings, "interval", 5);
}

Settings::Settings()
{
    // create config directory if needed
    BPtr<char> path = obs_module_get_config_path(obs_current_module(), "");
    os_mkdirs(path.Get());
}

// what
// ==85445== 64 bytes in 1 blocks are definitely lost in loss record 2,714 of 3,855
// ==85445==    at 0x483E7C5: malloc (vg_replace_malloc.c:380)
// ==85445==    by 0x6C169EC: a_malloc (bmem.c:46)
// ==85445==    by 0x6C16AAF: bmalloc (bmem.c:100)
// ==85445==    by 0x6C4CDBD: bzalloc (bmem.h:48)
// ==85445==    by 0x6C4CDBD: obs_data_array_create (obs-data.c:1368)
// ==85445==    by 0x6C4EE91: obs_data_add_json_array (obs-data.c:499)
// ==85445==    by 0x6C4EE91: obs_data_add_json_item (obs-data.c:525)
// ==85445==    by 0x6C4F092: obs_data_add_json_object_data (obs-data.c:482)
// ==85445==    by 0x6C4F092: obs_data_create_from_json (obs-data.c:641)
// ==85445==    by 0x2D3BA904: Settings::getSettings() (settings.cpp:29)
// ==85445==    by 0x2D3BB281: eventLoop(std::shared_ptr<Settings> const&) (source.cpp:28)
// ==85445==    by 0x79C83C3: execute_native_thread_routine (thread.cc:82)
// ==85445==    by 0x64E2258: start_thread (in /usr/lib/libpthread-2.33.so)
// ==85445==    by 0x7D685E2: clone (in /usr/lib/libc-2.33.so)
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

        cachedSettings = std::move(settings);
        cacheDirty = false;
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
