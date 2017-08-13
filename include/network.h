#ifndef KONSTRUCTS_NETWORK_H
#define KONSTRUCTS_NETWORK_H

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
#include "client.h"
#include "chunk_shader.h"
#include "sky_shader.h"
#include "selection_shader.h"
#include "hud.h"
#include "hud_shader.h"
#include "player_shader.h"
#include "textures.h"
#include "util.h"
#include "settings.h"

namespace konstructs {
    class Network {
    public:
        Network(Settings settings);
        void handle_network(Player player,
                            ChunkModelFactory *model_factory,
                            World world,
                            int radius,
                            uint32_t frame,
                            Hud hud,
                            GLFWwindow *mGLFWWindow,
                            PlayerShader *player_shader,
                            Vector3i player_chunk,
                            BlockTypeInfo blocks);
        bool setup_connection(Settings::Server server, GLFWwindow *mGLFWWindow);
        Client* get_client();

    private:
        void handle_packet(konstructs::Packet *packet,
                           Hud hud,
                           GLFWwindow *mGLFWWindow,
                           PlayerShader *player_shader,
                           Player player,
                           Vector3i player_chunk,
                           int radius,
                           BlockTypeInfo blocks);
        void handle_player_packet(const string &str,
                                  Player player,
                                  Vector3i player_chunk,
                                  int radius);
        void handle_other_player_packet(const string &str,
                                        PlayerShader *player_shader);
        void handle_delete_other_player_packet(const string &str,
                                               PlayerShader *player_shader);
        void handle_block_type(const string &str, BlockTypeInfo blocks);
        void handle_texture(konstructs::Packet *packet);
        void handle_belt(const string &str, Hud hud);
        void handle_inventory(const string &str, Hud hud);
        void handle_held_stack(const string &str, Hud hud);
        void handle_time(const string &str);

        Client client;
    };
};

#endif //KONSTRUCTS_NETWORK_H
