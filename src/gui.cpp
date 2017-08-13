#include "gui.h"

using Eigen::Vector2i;
using Eigen::Vector2f;
using nanogui::Screen;
using namespace konstructs;

GUI::GUI(Settings settings) :
        Screen(Eigen::Vector2i(settings.client.window_width,
                               settings.client.window_height),
               "Konstructs"),
        konstructs_data(settings),
        menu_state(false),
        settings(settings),
        network(settings) {

    performLayout(mNVGContext);
    show_pointer(false);
    Settings::Server server = settings.server;
    if (server.username.size() > 0 && server.password.size() > 0 && server.address.size() > 0) {
        connect();
    } else {
        show_menu(KONSTRUCTS_GUI_MENU_NORMAL, string("Connect to a server"));
    }

}

bool GUI::scrollEvent(const Vector2i &p, const Vector2f &rel) {
    // TODO
}

bool GUI::mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers) {
    return nanogui::Screen::mouseButtonEvent(p, button, down, modifiers);
}

bool GUI::keyboardEvent(int key, int scancode, int action, int modifiers) {
    // TODO
}

void GUI::draw(NVGcontext *ctx) {
    Screen::draw(ctx);
}

void GUI::drawContents() {
    // TODO
}

void GUI::show_menu(int state, string message) {
    using namespace nanogui;

    show_pointer(true);
    glActiveTexture(GL_TEXTURE0);

    FormHelper *gui = new FormHelper(this);
    window = gui->addWindow({0, 0}, "Main Menu");
    gui->setFixedSize({125, 20});

    if (state == KONSTRUCTS_GUI_MENU_POPUP) {
        // Popup message

        auto dlg = new MessageDialog(this, MessageDialog::Type::Warning, "Server connection", message);
    } else if (state == KONSTRUCTS_GUI_MENU_RECONNECT) {
        // Popup message with connect/cancel buttons.

        auto dlg = new MessageDialog(this, MessageDialog::Type::Warning,
                                     "Server connection", message,
                                     "Reconnect", "Cancel", true);
        dlg->setCallback([&](int result) {
            if (result == 0) {
                window->dispose();
                menu_state = false;
                connect();
            }
        });
    }

    gui->addVariable("Server address", settings.server.address);
    gui->addVariable("Username", settings.server.username);
    gui->addVariable("Password", settings.server.password);
    gui->addButton("Connect", [&]() {
        if (settings.server.username != "" &&
            settings.server.password != "" &&
            settings.server.address != "") {
            window->dispose();
            menu_state = false;
            if (connect()) {
                save_settings(settings);
            }
        }
    });

    window->center();
    performLayout(mNVGContext);
    menu_state = true;
}

void GUI::show_pointer(bool state) {
    if (state) {
        glfwSetInputMode(mGLFWWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    } else {
        glfwSetInputMode(mGLFWWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}

bool GUI::connect() {
    if (network.setup_connection(settings.server, mGLFWWindow)) {
        show_pointer(false);
        return true;
    } else {
        show_menu(KONSTRUCTS_GUI_MENU_RECONNECT, "Error: Connection failed");
        return false;
    }
}