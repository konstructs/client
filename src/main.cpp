
#include <nanogui/nanogui.h>
#if defined(WIN32)
    #define _WINSOCKAPI_
    #include <windows.h>
    #include <winsock2.h>
#endif
#include <nanogui/glutil.h>
#include <iostream>
#include <iomanip>
#include <memory>
#include <konstructs.h>

#define MAX_PENDING_CHUNKS 64

using namespace konstructs;

void print_usage();

void glfw_error(int error_code, const char *error_string);

#ifdef WIN32
int init_winsock() {
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    wVersionRequested = MAKEWORD(2, 2);
    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        printf("WSAStartup failed with error: %d\n", err);
        return 1;
    }

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        printf("Could not find a usable version of Winsock.dll\n");
        WSACleanup();
        return 1;
    }

    return 0;
}
#else
int init_winsock() {
    return 0;
}
#endif

void print_usage() {
    printf("OPTIONS: -h/--help                  - Show this help\n");
    printf("         -s/--server   <address>    - Server to enter\n");
    printf("         -u/--username <username>   - Username to login\n");
    printf("         -p/--password <password>   - Passworld to login\n\n");
    exit(0);
}

void glfw_error(int error_code, const char *error_string) {
    cout << "GLFW Error[" << error_code << "]: " << error_string << endl;
}

int main(int argc, char ** argv) {

    Settings settings;
    load_settings(settings);
    save_settings(settings);

    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
                print_usage();
            }
            if (strcmp(argv[i], "--server") == 0 || strcmp(argv[i], "-s") == 0) {
                if (!argv[i+1]) {
                    print_usage();
                } else {
                    settings.server.address = argv[i+1];
                    ++i;
                }
            }
            if (strcmp(argv[i], "--username") == 0 || strcmp(argv[i], "-u") == 0) {
                if (!argv[i+1]) {
                    print_usage();
                } else {
                    settings.server.username = argv[i+1];
                    ++i;
                }
            }
            if (strcmp(argv[i], "--password") == 0 || strcmp(argv[i], "-p") == 0) {
                if (!argv[i+1]) {
                    print_usage();
                } else {
                    settings.server.password = argv[i+1];
                    ++i;
                }
            }
            if (strcmp(argv[i], "--debug") == 0 || strcmp(argv[i], "-d") == 0) {
                settings.client.debug = true;
            }
        }

    }

    if (init_winsock()) {
        printf("Failed to load winsock");
        return 1;
    }

    try {
        glfwSetErrorCallback(glfw_error);
        nanogui::init();

        {
            nanogui::ref<Konstructs> app = new Konstructs(settings);
            app->drawAll();
            app->setVisible(true);
            nanogui::mainloop();
        }

        nanogui::shutdown();
    } catch (const std::runtime_error &e) {
        std::string error_msg = std::string("Caught a fatal error: ") + std::string(e.what());
        #if defined(WIN32)
        MessageBoxA(nullptr, error_msg.c_str(), NULL, MB_ICONERROR | MB_OK);
        #else
        std::cerr << error_msg << std::endl;
        #endif
        return -1;
    }

    #ifdef WIN32
    WSACleanup();
    #endif
    return 0;
}
