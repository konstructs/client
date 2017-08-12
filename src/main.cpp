#include <nanogui/nanogui.h>
#include <nanogui/glutil.h>
#include <iostream>
#include <konstructs.h>
#include "platform.h"
#include "cli.h"

using namespace konstructs;

void glfw_error(int error_code, const char *error_string) {
    cout << "GLFW Error[" << error_code << "]: " << error_string << endl;
}

int main(int argc, char **argv) {

    Settings settings;
    load_settings(settings);
    save_settings(settings);

    Cli::argument_parser(argc, argv, settings);

    if (Platform::init_winsock()) {
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
        #ifdef WIN32
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
