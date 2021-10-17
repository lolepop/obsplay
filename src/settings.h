#pragma once

#include <obs.hpp>
#include <mutex>

// struct InternalSettings
// {
//     bool enabled;
//     std::unordered_set<std::string> whitelist;
// };

class Settings
{
    std::mutex mtx;
    OBSData cachedSettings;
    bool cacheDirty = true;
    void refreshCache();

public:
    static void setDefaultSettings(obs_data_t* settings);
    static OBSDataArray getArray(obs_data_t* settings, const char* key);

    Settings();
    OBSData getSettings();
    void saveSettings();
};

// namespace Settings
// {
//     void setDefaultSettings(obs_data_t* settings);
//     OBSData getSettings();
//     void saveSettings(obs_data_t* settings);
//     OBSDataArray getArray(obs_data_t* settings, const char* key);

//     struct InternalSettings
//     {
//         bool enabled;
//         std::unordered_set<std::string> whitelist;
//     };
// }
