#ifndef KONSTRUCTS_KONSTRUCTS_H
#define KONSTRUCTS_KONSTRUCTS_H

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-attributes"
#include <nanogui/nanogui.h>
#pragma GCC diagnostic pop

#if defined(WIN32)
#define _WINSOCKAPI_
    #include <windows.h>
    #include <winsock2.h>
#else
#include <arpa/inet.h>
#endif
#include <nanogui/glutil.h>
#include <iostream>
#include <iomanip>
#include <memory>
#include "tiny_obj_loader.h"
#include "optional.hpp"
#include "matrix.h"
#include "shader.h"
#include "crosshair_shader.h"
#include "block.h"
#include "chunk.h"
#include "world.h"
#include "chunk_shader.h"
#include "sky_shader.h"
#include "selection_shader.h"
#include "hud.h"
#include "hud_shader.h"
#include "player_shader.h"
#include "textures.h"
#include "util.h"
#include "settings.h"
#include "network.h"

#define KONSTRUCTS_APP_TITLE "Konstructs"
#define MOUSE_CLICK_DELAY_IN_FRAMES 15

namespace konstructs {
    class Konstructs: public nanogui::Screen {
    public:

        Konstructs(Settings settings);
        ~Konstructs();

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

        // Members that are called from gui.cpp
        Hud hud;
        HudShader hud_shader;

    private:

        /** This function uses nanovg to print text on top of the screen. This is
         *  used for both the debug screen and messages sent from the server.
         */
        void print_top_text();

        /**
         * Update view distance variable dependent on your framerate.
         */
        bool update_view_distance();

        /**
         * If view distance has updated, set a new view radius.
         */
        void update_radius();

        /**
         * Manage the mouse pointer.
         */
        void handle_mouse();

        /**
         * Manage the keyboard.
         */
        void handle_keys();

        /**
         * Close the hud and send a message to the server.
         */
        void close_hud();

        /**
         * Returns the time of day
         */
        float time_of_day();

        /**
         * TODO: What do this?
         */
        float daylight();

        /**
         * Open and display the nanogui main menu
         */
        void show_menu(int state, string message);

        BlockTypeInfo blocks;
        CrosshairShader crosshair_shader;
        int radius;
        float view_distance;
        float near_distance;
        int day_length;
        World world;
        SkyShader sky_shader;
        ChunkShader chunk_shader;
        SelectionShader selection_shader;
        PlayerShader *player_shader;
        ChunkModelFactory model_factory;
        Player player;
        Vector3i player_chunk;
        optional<pair<konstructs::Block, konstructs::Block>> looking_at;
        double px;
        double py;
        FPS fps;
        double last_frame;
        bool debug_text_enabled;
        nanogui::Window *window;
        uint32_t frame;
        uint32_t faces;
        uint32_t max_faces;
        double frame_time;
        uint32_t click_delay;
        Settings settings;
        Network network;
    };
};

#endif //KONSTRUCTS_KONSTRUCTS_H
