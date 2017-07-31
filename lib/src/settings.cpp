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
            ini.SaveFile(settings_path);
        }
    }
}