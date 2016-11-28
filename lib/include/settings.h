#ifndef KONSTRUCTS_SETTINGS_H
#define KONSTRUCTS_SETTINGS_H

#include <string>

namespace konstructs {
    class Settings {
    public:
        virtual std::string get_conf_string(std::string group, std::string key, std::string def_val) = 0;
        virtual bool get_conf_boolean(std::string group, std::string key, bool def_val) = 0;
        virtual void save() = 0;
    };

    Settings * load_settings(std::string file_path);
};

#endif //KONSTRUCTS_SETTINGS_H
