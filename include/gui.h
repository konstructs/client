#ifndef KONSTRUCTS_GUI_H
#define KONSTRUCTS_GUI_H

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-attributes"
#include <nanogui/nanogui.h>
#pragma GCC diagnostic pop

#include "settings.h"
#include "konstructs.h"

using Eigen::Vector2i;
using Eigen::Vector2f;
using nanogui::Screen;

#define KONSTRUCTS_GUI_MENU_NORMAL 0
#define KONSTRUCTS_GUI_MENU_POPUP 1
#define KONSTRUCTS_GUI_MENU_RECONNECT 2

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

    private:

        /**
         * Display the main menu
         * @param state     KONSTRUCTS_GUI_MENU_NORMAL = No extra popups
         *                  KONSTRUCTS_GUI_MENU_POPUP = A popup message
         *                  KONSTRUCTS_GUI_MENU_RECONNECT = Server connection retry dialog
         * @param message   A message to be displayed
         */
        void show_menu(int state, string message);

        /**
         * Holds various konstructs data like for example shaders
         * and the chunk processing facility. This is mainly here
         * for legacy reasons and may be removed in the future.
         */
        Konstructs konstructs_data;

        /**
         * Show or hide the mouse pointer.
         * @param state     Mouse pointer state
         */
        void show_pointer(bool state);

        bool menu_state;
        Settings settings;
        nanogui::Window *window;
    };
}


#endif //KONSTRUCTS_GUI_H
