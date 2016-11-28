#ifndef KONSTRUCTS_SETTINGS_IMPL_H
#define KONSTRUCTS_SETTINGS_IMPL_H

#include <string>
#include <SimpleIni.h>
#include "settings.h"

namespace konstructs {
    class SettingsImpl : public Settings {
    public:
        SettingsImpl(const std::string settings_file);
        std::string get_conf_string(std::string group, std::string key, std::string def_val);
        bool get_conf_boolean(std::string group, std::string key, bool def_val);
        void save();

    private:
        CSimpleIniA ini;
        std::string settings_file_path;

        std::string conf_hostname;
        std::string conf_username;
        std::string conf_password;
    };
};


#endif //KONSTRUCTS_SETTINGS_IMPL_H
