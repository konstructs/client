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
        settings(settings) {

    performLayout(mNVGContext);
    //glfwSetInputMode(mGLFWWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    Settings::Server server = settings.server;
    if (server.username.size() > 0 && server.password.size() > 0 && server.address.size() > 0) {
        //network.setup_connection(server, mGLFWWindow);
    } else {
        show_menu(0, string("Connect to a server"));
    }

}

bool GUI::scrollEvent(const Vector2i &p, const Vector2f &rel) {
    // TODO
}

bool GUI::mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers) {
    // TODO
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

    glfwSetInputMode(mGLFWWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glActiveTexture(GL_TEXTURE0);

    FormHelper *gui = new FormHelper(this);
    Window *window = gui->addWindow({0, 0}, "Main Menu");
    gui->setFixedSize({125, 20});

    if (state == 1) {
        // Popup message

        auto dlg = new MessageDialog(this, MessageDialog::Type::Warning, "Server connection", message);
    } else if (state == 2) {
        // Popup message with connect/cancel buttons.

        auto dlg = new MessageDialog(this, MessageDialog::Type::Warning,
                                     "Server connection", message,
                                     "Reconnect", "Cancel", true);
        dlg->setCallback([&](int result) {
            if (result == 0) {
                window->dispose();
                menu_state = false;
                //network.setup_connection(settings.server, mGLFWWindow);
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
            // Note: The mouse pointer is intentionally not locked here.
            // See: setup_connection()
            window->dispose();
            menu_state = false;
            // network.setup_connection(settings.server, mGLFWWindow);
            // save_settings(settings);
        }
    });

    window->center();
    performLayout(mNVGContext);
    menu_state = true;
}