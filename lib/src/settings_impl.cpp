#include <iostream>
#include "settings_impl.h"

namespace konstructs {

    SettingsImpl::SettingsImpl(std::string settings_file) {
        ini.SetUnicode();
        ini.LoadFile(settings_file.c_str());
        settings_file_path = settings_file;
    }

    std::string SettingsImpl::get_conf_string(std::string group, std::string key, std::string def_val) {
        const char * value = ini.GetValue(group.c_str(), key.c_str(), def_val.c_str());
        return std::string(value);
    }

    bool SettingsImpl::get_conf_boolean(std::string group, std::string key, bool def_val) {
        return ini.GetBoolValue(group.c_str(), key.c_str(), def_val);
    }

    void SettingsImpl::save() {
        ini.Save(settings_file_path);
    }

    Settings * load_settings(std::string file_path) {
        return new SettingsImpl(file_path);
    }
}
