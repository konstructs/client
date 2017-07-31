#include <iostream>

#ifndef KONSTRUCTS_SETTINGS_H
#define KONSTRUCTS_SETTINGS_H

namespace konstructs {
    struct Settings {
        struct Server {
            std::string address;
            unsigned int port;
            std::string username;
            std::string password;
            bool password_save;
        };

        struct Client {
            bool debug;
        };

        Server server;
        Client client;
    };

    /**
     * Uses simpleini to populate the Settings struct from a ini file
     * @param A reference to the settings object
     */
    void load_settings(Settings &settings);

    /**
     * Uses simpleini to save the Settings struct to a ini file
     * @param A reference to the settings object
     */
    void save_settings(Settings &settings);
}

#endif //KONSTRUCTS_SETTINGS_H