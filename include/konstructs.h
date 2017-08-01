#ifndef KONSTRUCTS_KONSTRUCTS_H
#define KONSTRUCTS_KONSTRUCTS_H

#include <nanogui/nanogui.h>
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

#define KONSTRUCTS_APP_TITLE "Konstructs"
#define MOUSE_CLICK_DELAY_IN_FRAMES 15

namespace konstructs {
    class Konstructs: public nanogui::Screen {
    public:

        Konstructs(Settings settings);
        ~Konstructs();
        virtual bool scrollEvent(const Vector2i &p, const Vector2f &rel);
        virtual bool mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers);
        virtual bool keyboardEvent(int key, int scancode, int action, int modifiers);
        virtual void draw(NVGcontext *ctx);
        virtual void drawContents();

    private:

        /** This function uses nanovg to print text on top of the screen. This is
         *  used for both the debug screen and messages sent from the server.
         */
        void print_top_text();
        int translate_button(int button);
        bool update_view_distance();
        void update_radius();
        void handle_mouse();
        void handle_keys();
        void close_hud();
        void handle_network();
        void handle_packet(konstructs::Packet *packet);
        void handle_player_packet(const string &str);
        void handle_other_player_packet(const string &str);
        void handle_delete_other_player_packet(const string &str);
        void handle_block_type(const string &str);
        void handle_texture(konstructs::Packet *packet);
        void handle_belt(const string &str);
        void handle_inventory(const string &str);
        void handle_held_stack(const string &str);
        void handle_time(const string &str);
        float time_of_day();
        float daylight();
        void show_menu(int state, string message);
        void setup_connection();

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
        HudShader hud_shader;
        PlayerShader *player_shader;
        ChunkModelFactory model_factory;
        Client client;
        Player player;
        Vector3i player_chunk;
        optional<pair<konstructs::Block, konstructs::Block>> looking_at;
        Hud hud;
        double px;
        double py;
        FPS fps;
        double last_frame;
        bool menu_state;
        bool debug_text_enabled;
        nanogui::Window *window;
        uint32_t frame;
        uint32_t faces;
        uint32_t max_faces;
        double frame_time;
        uint32_t click_delay;
        Settings settings;
    };
};

#endif //KONSTRUCTS_KONSTRUCTS_H
