#ifndef KONSTRUCTS_GUI_H
#define KONSTRUCTS_GUI_H

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-attributes"
#include <nanogui/nanogui.h>
#pragma GCC diagnostic pop

#include "settings.h"

using Eigen::Vector2i;
using Eigen::Vector2f;
using nanogui::Screen;

namespace konstructs {

    class GUI: public nanogui::Screen {
    public:

        GUI(Settings settings);

        /**
         * Called by GLFW when the mouse scroll wheel is used.
         */
        virtual bool scrollEvent(const Vector2i &p, const Vector2f &rel);

        /**
         * Called by GLFW when a mouse button is pressed
         */
        virtual bool mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers);

        /**
         * Called by GLEW when a keyboard button is pressed
         */
        virtual bool keyboardEvent(int key, int scancode, int action, int modifiers);

        virtual void draw(NVGcontext *ctx);
        virtual void drawContents();
    };
}


#endif //KONSTRUCTS_GUI_H
