#include <iostream>
#include <settings.h>
#include <cstring>
#include "cli.h"

using namespace konstructs;

void Cli::print_usage() {
    std::cout << ("OPTIONS: -h/--help                  - Show this help") << std::endl;
    std::cout << ("         -s/--server   <address>    - Server to enter")  << std::endl;
    std::cout << ("         -u/--username <username>   - Username to login")  << std::endl;
    std::cout << ("         -p/--password <password>   - Passworld to login")  << std::endl;
    exit(0);
}

void Cli::argument_parser(int argc, char **argv, Settings *settings) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            Cli::print_usage();
        }
        if (strcmp(argv[i], "--server") == 0 || strcmp(argv[i], "-s") == 0) {
            if (!argv[i+1]) {
                Cli::print_usage();
            } else {
                settings->server.address = argv[i+1];
                ++i;
            }
        }
        if (strcmp(argv[i], "--username") == 0 || strcmp(argv[i], "-u") == 0) {
            if (!argv[i+1]) {
                Cli::print_usage();
            } else {
                settings->server.username = argv[i+1];
                ++i;
            }
        }
        if (strcmp(argv[i], "--password") == 0 || strcmp(argv[i], "-p") == 0) {
            if (!argv[i+1]) {
                Cli::print_usage();
            } else {
                settings->server.password = argv[i+1];
                ++i;
            }
        }
        if (strcmp(argv[i], "--debug") == 0 || strcmp(argv[i], "-d") == 0) {
            settings->client.debug = true;
        }
    }
}