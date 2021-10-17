#include "source.h"
#include "obsplayui.h"
#include "settings.h"
#include "native.h"

OBS_DECLARE_MODULE()

std::vector<std::string> getWindowFilter(obs_data_array_t* data)
{
    auto out = std::vector<std::string>();
    for (size_t i = 0; i < obs_data_array_count(data); i++)
    {
        obs_data_t* item = obs_data_array_item(data, i);
        auto p = obs_data_get_string(item, "process");
        out.push_back(p);
		obs_data_release(item);
    }
    return out;
}

void eventLoop(const std::shared_ptr<Settings>& settings)
{
    int idleTime = 0;
    auto lastScan = std::chrono::high_resolution_clock::now();

    while (true)
    {
        auto s = settings->getSettings();
        auto isEnabled = obs_data_get_bool(s, "enabled");

        if (isEnabled)
        {
            auto replayBufferActive = obs_frontend_replay_buffer_active();
            auto windowFilter = getWindowFilter(Settings::getArray(s, "whitelist"));
            auto timeout = obs_data_get_int(s, "timeout");

            if (Native::isWindowOpen(windowFilter))
            {
                idleTime = 0;
                if (!replayBufferActive)
                {
                    obs_frontend_replay_buffer_start();
                    log("replay buffer started");
                }
            }
            else
            {
                idleTime += std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - lastScan).count();
                if (idleTime >= timeout && replayBufferActive)
                {
                    obs_frontend_replay_buffer_stop();
                    log("replay buffer stopped");
                }
            }

        }

        lastScan = std::chrono::high_resolution_clock::now();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

void initUI(const std::shared_ptr<Settings>& settings)
{
    auto action = reinterpret_cast<QAction*>(obs_frontend_add_tools_menu_qaction("Obsplay"));

	action->connect(action, &QAction::triggered, [=]() {
        auto window = reinterpret_cast<QMainWindow*>(obs_frontend_get_main_window());
	    ObsPlayUI cfgUI(settings, window);
        cfgUI.exec();
    });
}

bool obs_module_load(void)
{
    auto settings = std::make_shared<Settings>();
    initUI(settings);

    log("started");

    std::thread t(eventLoop, settings);
    t.detach();

    log("started event thread");

    return true;
}