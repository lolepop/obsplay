#pragma once

#include <obs-module.h>
#include <obs-frontend-api.h>
#include <obs.hpp>
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <unordered_set>
#include <memory>
#include <chrono>
#include <QMainWindow>
#include <QAction>

#if defined(__linux) || defined(__linux__) || defined(linux)
    #define LINUX
#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(_WIN64)
    #define WINDOWS
#endif

#define SETTINGS_PATH() obs_module_get_config_path(obs_current_module(), "settings.json")

#define log(s) blog(LOG_INFO, "obsplay: %s", s);
