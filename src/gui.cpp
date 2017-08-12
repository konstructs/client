#include "gui.h"

using Eigen::Vector2i;
using Eigen::Vector2f;
using nanogui::Screen;
using namespace konstructs;

GUI::GUI(Settings settings) :
        nanogui::Screen(Eigen::Vector2i(settings.client.window_width,
                                        settings.client.window_height),
                        "Konstructs") {}

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
