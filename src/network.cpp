#include "network.h"

using std::cout;
using std::cerr;
using std::endl;
using namespace konstructs;
using nonstd::optional;
using nonstd::nullopt;
using std::pair;

Network::Network(Settings settings) : client(settings.client.debug) {}

void Network::handle_network(Player player,
                             ChunkModelFactory *model_factory,
                             World world,
                             int radius,
                             uint32_t frame,
                             Hud hud,
                             GLFWwindow *mGLFWWindow,
                             PlayerShader *player_shader,
                             Vector3i player_chunk,
                             BlockTypeInfo blocks) {
    for (auto packet : client.receive(100)) {
        handle_packet(packet.get(), hud, mGLFWWindow, player_shader, player, player_chunk, radius, blocks);
    }
    Vector3f pos = player.position;
    Vector3i _player_chunk(chunked(pos[0]), chunked(pos[2]), chunked(pos[1]));

    auto prio = client.receive_prio_chunk(_player_chunk);

    model_factory->update_player_chunk(_player_chunk);
    /* Insert prio chunk into world */
    if (prio) {
        world.insert(*prio);
        model_factory->create_models({(*prio).position}, world);
    }
    auto new_chunks = client.receive_chunks(1);
    if (!new_chunks.empty()) {
        std::vector<Vector3i> positions;
        positions.reserve(new_chunks.size());
        for (auto chunk : new_chunks) {
            world.insert(chunk);
            positions.push_back(chunk.position);
        }
        model_factory->create_models(positions, world);
    }
    if (frame % 7883 == 0) {
        /* Book keeping */
        world.delete_unused_chunks(_player_chunk, radius + KEEP_EXTRA_CHUNKS);
    }

}

void Network::handle_packet(konstructs::Packet *packet,
                            Hud hud,
                            GLFWwindow *mGLFWWindow,
                            PlayerShader *player_shader,
                            Player player,
                            Vector3i player_chunk,
                            int radius,
                            BlockTypeInfo blocks) {
    switch (packet->type) {
        case 'P':
            handle_other_player_packet(packet->to_string(), player_shader);
            break;
        case 'D':
            handle_delete_other_player_packet(packet->to_string(), player_shader);
            break;
        case 'U':
            handle_player_packet(packet->to_string(), player, player_chunk, radius);
            break;
        case 'W':
            handle_block_type(packet->to_string(), blocks);
            break;
        case 'M':
            handle_texture(packet);
            break;
        case 'G':
            handle_belt(packet->to_string(), hud);
            break;
        case 'I':
            handle_inventory(packet->to_string(), hud);
            hud.set_interactive(true);
            glfwSetInputMode(mGLFWWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            break;
        case 'i':
            handle_held_stack(packet->to_string(), hud);
            break;
        case 'T':
            handle_time(packet->to_string());
            break;
        default:
            cout << "UNKNOWN: " << packet->type << endl;
            break;
    }
}

void Network::handle_player_packet(const string &str,
                                   Player player,
                                   Vector3i player_chunk,
                                   int radius) {
    int pid;
    float x, y, z, rx, ry;

    if (sscanf(str.c_str(), ",%d,%f,%f,%f,%f,%f",
               &pid, &x, &y, &z, &rx, &ry) != 6) {
        throw std::runtime_error(str);
    }
    player = Player(pid, Vector3f(x, y, z), rx, ry);
    player_chunk = chunked_vec(player.camera());
    client.set_player_chunk(player_chunk);
    client.set_radius(radius);
    client.set_logged_in(true);
}

void Network::handle_other_player_packet(const string &str,
                                         PlayerShader *player_shader) {
    int pid;
    float x, y, z, rx, ry;
    if (sscanf(str.c_str(), ",%d,%f,%f,%f,%f,%f",
               &pid, &x, &y, &z, &rx, &ry) != 6) {
        throw std::runtime_error(str);
    }
    player_shader->add(Player(pid, Vector3f(x, y, z), rx, ry));
}

void Network::handle_delete_other_player_packet(const string &str,
                                                PlayerShader *player_shader) {
    int pid;

    if (sscanf(str.c_str(), ",%d",
               &pid) != 1) {
        throw std::runtime_error(str);
    }
    player_shader->remove(pid);
}

void Network::handle_block_type(const string &str, BlockTypeInfo blocks) {
    int w, obstacle, transparent, left, right, top, bottom, front, back, orientable;
    char shape[16];
    char state[16];
    if (sscanf(str.c_str(), ",%d,%15[^,],%15[^,],%d,%d,%d,%d,%d,%d,%d,%d,%d",
               &w, shape, state, &obstacle, &transparent, &left, &right,
               &top, &bottom, &front, &back, &orientable) != 12) {
        throw std::runtime_error(str);
    }
    blocks.is_plant[w] = strncmp(shape, "plant", 16) == 0;
    if (strncmp(state, "solid", 16) == 0) {
        blocks.state[w] = STATE_SOLID;
    } else if (strncmp(state, "liquid", 16) == 0) {
        blocks.state[w] = STATE_LIQUID;
    } else if (strncmp(state, "gas", 16) == 0) {
        blocks.state[w] = STATE_GAS;
    } else if (strncmp(state, "plasma", 16) == 0) {
        blocks.state[w] = STATE_PLASMA;
    } else {
        throw std::invalid_argument("Invalid block type state received!");
    }
    blocks.is_obstacle[w] = obstacle;
    blocks.is_transparent[w] = transparent;
    blocks.is_orientable[w] = orientable;
    blocks.blocks[w][0] = left;
    blocks.blocks[w][1] = right;
    blocks.blocks[w][2] = top;
    blocks.blocks[w][3] = bottom;
    blocks.blocks[w][4] = front;
    blocks.blocks[w][5] = back;
}

void Network::handle_texture(konstructs::Packet *packet) {
    GLuint texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0 + BLOCK_TEXTURES);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    load_png_texture_from_buffer(packet->buffer(), packet->size);
}

void Network::handle_belt(const string &str, Hud hud) {
    uint32_t column, size, type, health;
    if (sscanf(str.c_str(), ",%u,%u,%u,%u",
               &column, &size, &type, &health) != 4) {
        throw std::runtime_error(str);
    }

    if (size < 1) {
        hud.reset_belt(column);
    } else {
        hud.set_belt(column, {size, (uint16_t) type, (uint16_t) health});
    }
}

void Network::handle_inventory(const string &str, Hud hud) {
    uint32_t index, size, type, health;
    if (sscanf(str.c_str(), ",%u,%u,%u,%u",
               &index, &size, &type, &health) != 4) {
        throw std::runtime_error(str);
    }
    uint32_t row = index / 17;
    uint32_t column = index % 17;
    Vector2i pos(column, row);

    if (type == -1) {
        hud.reset_background(pos);
        hud.reset_stack(pos);
    } else {
        hud.set_background(pos, 2);
        hud.set_stack(pos, {size, (uint16_t) type, (uint16_t) health});
    }
}

void Network::handle_held_stack(const string &str, Hud hud) {
    uint32_t amount, type;
    if (sscanf(str.c_str(), ",%u,%u",
               &amount, &type) != 2) {
        throw std::runtime_error(str);
    }
    if (type == -1) {
        hud.reset_held();
    } else {
        hud.set_held({amount, (uint16_t) type});
    }
}

void Network::handle_time(const string &str) {
    long time_value;
    if (sscanf(str.c_str(), ",%lu", &time_value) != 1) {
        throw std::runtime_error(str);
    }
    glfwSetTime((double) time_value);
}

bool Network::setup_connection(Settings::Server server, GLFWwindow *mGLFWWindow) {
    try {
        client.open_connection(server);
        /* TODO: We do not really know if the server accepted our protocol
                 version, or our username/password combo here. set_connected
                 will release a lock and start to receive data, and fail. */
        load_textures();
        client.set_connected(true);
        return true;
    } catch (const std::exception &ex) {
        std::cerr << client.get_error_message() << std::endl;
        return false;
    }
}

Client* Network::get_client() {
    return &client;
}