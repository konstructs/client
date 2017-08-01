#include "settings.h"
#include "SimpleIni.h"
#include "GLFW/glfw3.h"
#include <sys/stat.h>

namespace konstructs {

    void config_path(char* r, size_t size) {
        if(const char* path = std::getenv("LOCALAPPDATA")) {
            // Looks like Windows, C:\Users\(user-name)\AppData\Local
            snprintf(r, size, "%s\\%s", path, "konstructs.ini");
        } else if (const char* path = std::getenv("SNAP_USER_DATA")) {
            // Looks like Linux and inside a snap, $HOME/snap/konstructs-client/(version)
            snprintf(r, size, "%s/%s", path, "konstructs.conf");
        } else if (const char* path = std::getenv("HOME")) {
            char _path[4096];
            snprintf(_path, 4096, "%s/%s", path, ".config");
            struct stat info;
            if (stat(_path, &info) == 0) {
                // Looks like we have a $HOME/.config folder, use it
                snprintf(r, size, "%s/%s", _path, "konstructs.conf");
            } else {
                // Just use a classic dotfile in HOME
                snprintf(r, size, "%s/%s", path, ".konstructs.conf");
            }
        }
    }

    void load_settings(Settings &settings) {
        char settings_path[4096];
        config_path(settings_path, 4096);

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
        settings.keys.up = (int)ini.GetLongValue("keys", "up", GLFW_KEY_W);
        settings.keys.down = (int)ini.GetLongValue("keys", "down", GLFW_KEY_S);
        settings.keys.left = (int)ini.GetLongValue("keys", "left", GLFW_KEY_A);
        settings.keys.right = (int)ini.GetLongValue("keys", "right", GLFW_KEY_D);
        settings.keys.jump = (int)ini.GetLongValue("keys", "jump", GLFW_KEY_SPACE);
        settings.keys.fly = (int)ini.GetLongValue("keys", "fly", GLFW_KEY_TAB);
        settings.keys.sneak = (int)ini.GetLongValue("keys", "sneak", GLFW_KEY_LEFT_SHIFT);
        settings.keys.tertiary = (int)ini.GetLongValue("keys", "tertiary", GLFW_KEY_E);
        settings.keys.debug = (int)ini.GetLongValue("keys", "debug", GLFW_KEY_F3);
    }

    void save_settings(Settings &settings) {
        char settings_path[4096];
        config_path(settings_path, 4096);

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
        ini.SetLongValue("keys", "up", settings.keys.up);
        ini.SetLongValue("keys", "down", settings.keys.down);
        ini.SetLongValue("keys", "left", settings.keys.left);
        ini.SetLongValue("keys", "right", settings.keys.right);
        ini.SetLongValue("keys", "jump", settings.keys.jump);
        ini.SetLongValue("keys", "fly", settings.keys.fly);
        ini.SetLongValue("keys", "sneak", settings.keys.sneak);
        ini.SetLongValue("keys", "tertiary", settings.keys.tertiary);
        ini.SetLongValue("keys", "debug", settings.keys.debug);
        ini.SaveFile(settings_path);
    }
}