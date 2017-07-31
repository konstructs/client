#include "settings.h"
#include "SimpleIni.h"

namespace konstructs {

    void load_settings(Settings &settings) {
        char settings_path[255];

        if(const char* env_home = std::getenv("HOME")) {
            snprintf(settings_path, 255, "%s/%s", env_home, ".konstructs.conf");

            CSimpleIniA ini;
            ini.SetUnicode(true);
            ini.LoadFile(settings_path);

            // Load default values (and set defaults)
            settings.server.address = ini.GetValue("server", "address", "play.konstructs.org");
            settings.server.port = (unsigned int)ini.GetLongValue("server", "port", 4080);
            settings.server.username = ini.GetValue("server", "username", "");
            settings.server.password = ini.GetValue("server", "password", "");
            settings.server.password_save = !settings.server.password.empty();
            settings.client.debug = ini.GetBoolValue("client", "debug", false);
            settings.client.field_of_view = (int)ini.GetLongValue("client", "field_of_view", 70);
            settings.client.window_width = (unsigned int)ini.GetLongValue("client", "window_width", 854);
            settings.client.window_height = (unsigned int)ini.GetLongValue("client", "window_height", 480);
            settings.client.radius_start = (unsigned int)ini.GetLongValue("client", "radius_start", 5);
            settings.client.radius_max = (unsigned int)ini.GetLongValue("client", "radius_max", 20);
            settings.client.frames_per_second = (float)ini.GetLongValue("client", "frames_per_second", 60);
        }
    }

    void save_settings(Settings &settings) {
        char settings_path[255];

        if(const char* env_home = std::getenv("HOME")) {
            snprintf(settings_path, 255, "%s/%s", env_home, ".konstructs.conf");

            CSimpleIniA ini;
            ini.SetUnicode(true);

            ini.SetValue("server", "address", settings.server.address.c_str());
            ini.SetLongValue("server", "port", settings.server.port);
            ini.SetValue("server", "username", settings.server.username.c_str());

            // Only save the password back to the file if it's already there.
            // This allows users to save the password, but it's an opt-in.
            if (settings.server.password_save) {
                ini.SetValue("server", "password", settings.server.password.c_str());
            } else {
                ini.SetValue("server", "password", "");
            }

            ini.SetBoolValue("client", "debug", settings.client.debug);
            ini.SetLongValue("client", "field_of_view", settings.client.field_of_view);
            ini.SetLongValue("client", "window_width", settings.client.window_width);
            ini.SetLongValue("client", "window_height", settings.client.window_height);
            ini.SetLongValue("client", "radius_start", settings.client.radius_start);
            ini.SetLongValue("client", "radius_max", settings.client.radius_max);
            ini.SetLongValue("client", "frames_per_second", (long)settings.client.frames_per_second);
            ini.SaveFile(settings_path);
        }
    }
}