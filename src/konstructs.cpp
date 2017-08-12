#include "konstructs.h"
#include "network.h"

using std::cout;
using std::cerr;
using std::endl;
using namespace konstructs;
using nonstd::optional;
using nonstd::nullopt;
using std::pair;


Konstructs::Konstructs(Settings settings) :
        nanogui::Screen(Eigen::Vector2i(settings.client.window_width,
                                        settings.client.window_height),
                        KONSTRUCTS_APP_TITLE),
        player(0, Vector3f(0.0f, 0.0f, 0.0f), 0.0f, 0.0f),
        px(0), py(0),
        model_factory(blocks),
        radius(settings.client.radius_start),
        view_distance((float) settings.client.radius_start * CHUNK_SIZE),
        near_distance(0.125f),
        sky_shader(settings.client.field_of_view, SKY_TEXTURE, near_distance),
        chunk_shader(settings.client.field_of_view, BLOCK_TEXTURES, DAMAGE_TEXTURE, SKY_TEXTURE, near_distance,
                     load_chunk_vertex_shader(), load_chunk_fragment_shader()),
        hud_shader(17, 14, INVENTORY_TEXTURE, BLOCK_TEXTURES, FONT_TEXTURE, HEALTH_BAR_TEXTURE),
        selection_shader(settings.client.field_of_view, near_distance, 0.52),
        day_length(600),
        last_frame(glfwGetTime()),
        looking_at(nullopt),
        hud(17, 14, 9),
        menu_state(false),
        debug_text_enabled(false),
        frame(0),
        click_delay(0),
        settings(settings),
        network(settings) {

    using namespace nanogui;
    performLayout(mNVGContext);
    glfwSetInputMode(mGLFWWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    Settings::Server server = settings.server;
    if (server.username.size() > 0 && server.password.size() > 0 && server.address.size() > 0) {
        network.setup_connection(server, mGLFWWindow);
    } else {
        show_menu(0, string("Connect to a server"));
    }
    blocks.is_plant[SOLID_TYPE] = 0;
    blocks.is_obstacle[SOLID_TYPE] = 1;
    blocks.is_transparent[SOLID_TYPE] = 0;
    blocks.state[SOLID_TYPE] = STATE_SOLID;
    memset(&fps, 0, sizeof(fps));

    tinyobj::shape_t shape = load_player();
    player_shader = new PlayerShader(settings.client.field_of_view, PLAYER_TEXTURE, SKY_TEXTURE,
                                     near_distance, shape);
}

Konstructs::~Konstructs() {
    delete player_shader;
}

bool Konstructs::scrollEvent(const Vector2i &p, const Vector2f &rel) {
    hud.scroll(rel[1]);
    return true;
}

bool Konstructs::mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers) {
    if (hud.get_interactive()) {
        if (down) {
            double x, y;
            glfwGetCursorPos(mGLFWWindow, &x, &y);

            auto clicked_at = hud_shader.clicked_at(x, y, mSize.x(), mSize.y());

            if (clicked_at) {
                Vector2i pos = *clicked_at;
                if (hud.active(pos)) {
                    int index = pos[0] + pos[1] * 17;
                    network.get_client()->click_inventory(index, translate_button(button));
                }
            }
        }
    } else if (!menu_state) {
        // Clicking at the window captures the mouse pointer
        glfwSetInputMode(mGLFWWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    return nanogui::Screen::mouseButtonEvent(p, button, down, modifiers);
}

bool Konstructs::keyboardEvent(int key, int scancode, int action, int modifiers) {
    if (nanogui::Screen::keyboardEvent(key, scancode, action, modifiers)) {
        return true;
    }
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        if (hud.get_interactive()) {
            close_hud();
        } else {
            glfwSetInputMode(mGLFWWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    } else if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        // TODO: implement this again when time has come ;)
        /*if (!menu_state) {
            show_menu("","","");
        } else {
            hide_menu();
        }*/
    } else if (key == settings.keys.debug && action == GLFW_PRESS) {
        debug_text_enabled = !debug_text_enabled;
    } else if (key == settings.keys.fly
               && action == GLFW_PRESS
               && settings.client.debug) {
        player.fly();
    } else if (key == settings.keys.tertiary && action == GLFW_PRESS) {
        if (hud.get_interactive()) {
            close_hud();
        } else if (network.get_client()->is_connected()) {
            if (looking_at) {
                auto &l = *looking_at;
                uint8_t direction = direction_from_vector(l.first.position, l.second.position);
                uint8_t rotation = rotation_from_vector(direction, player.camera_direction());
                network.get_client()->click_at(1, l.second.position, 3, hud.get_selection(), direction, rotation);
            } else {
                network.get_client()->click_at(0, Vector3i::Zero(), 3, hud.get_selection(), 0, 0);
            }
        }
    } else if (key > 48 && key < 58 && action == GLFW_PRESS) {
        hud.set_selected(key - 49);
    } else {
        return false;
    }
    return true;
}

void Konstructs::draw(NVGcontext *ctx) {
    Screen::draw(ctx);
}

void Konstructs::drawContents() {
    using namespace nanogui;
    update_fps(&fps);
    frame++;
    if (network.get_client()->is_connected()) {
        network.handle_network(player,
                               &model_factory,
                               world,
                               radius,
                               frame,
                               hud,
                               mGLFWWindow,
                               player_shader,
                               player_chunk,
                               blocks);
        handle_keys();
        handle_mouse();
        looking_at = player.looking_at(world, blocks);
        glClear(GL_DEPTH_BUFFER_BIT);
        for (auto model : model_factory.fetch_models()) {
            chunk_shader.add(model);
        }
        sky_shader.render(player, mSize.x(), mSize.y(), time_of_day(), view_distance);
        glClear(GL_DEPTH_BUFFER_BIT);
        faces = chunk_shader.render(player, mSize.x(), mSize.y(),
                                    daylight(), time_of_day(), radius,
                                    view_distance, player_chunk);
        if (faces > max_faces) {
            max_faces = faces;
        }
        player_shader->render(player, mSize.x(), mSize.y(),
                              daylight(), time_of_day(), view_distance);
        if (looking_at && !hud.get_interactive() && !menu_state) {
            selection_shader.render(player, mSize.x(), mSize.y(),
                                    looking_at->second.position, view_distance);
        }
        glClear(GL_DEPTH_BUFFER_BIT);
        if (!hud.get_interactive() && !menu_state) {
            crosshair_shader.render(mSize.x(), mSize.y());
        }
        double mx, my;
        glfwGetCursorPos(mGLFWWindow, &mx, &my);
        hud_shader.render(mSize.x(), mSize.y(), mx, my, hud, blocks);
        update_radius();
        print_top_text();
    } else if (!menu_state) {
        show_menu(2, network.get_client()->get_error_message());
    }
}

/** This function uses nanovg to print text on top of the screen. This is
 *  used for both the debug screen and messages sent from the server.
 */
void Konstructs::print_top_text() {
    int width, height;
    glfwGetFramebufferSize(mGLFWWindow, &width, &height);

    ostringstream os;
    if (debug_text_enabled) {
        double frame_fps = 1.15 / frame_time;
        os << std::fixed << std::setprecision(2);
        os << "Server: " << settings.server.address
           << " user: " << settings.server.username
           << " x: " << player.position(0)
           << " y: " << player.position(1)
           << " z: " << player.position(2)
           << std::endl;
        if (looking_at) {
            auto l = *looking_at;
            uint8_t direction = direction_from_vector(l.first.position, l.second.position);
            uint8_t rotation = rotation_from_vector(direction, player.camera_direction());
            os << "Pointing at x: " << l.second.position(0) << ", "
               << "y: " << l.second.position(1) << ", "
               << "z: " << l.second.position(2) << ", "
               << "dir: " << direction_to_string[direction] << ", "
               << "rot: " << rotation_to_string[rotation]
               << std::endl;
        } else {
            os << "Pointing at nothing." << std::endl;
        }
        os << "View distance: " << view_distance << " (" << radius << "/" << network.get_client()->get_loaded_radius() << ") "
           << "faces: " << faces << "(" << max_faces << ") "
           << "FPS: " << fps.fps << "(" << frame_fps << ")" << endl;
        os << "Chunks: " << world.size() << " "
           << "models: " << chunk_shader.size() << endl;
        os << "Model factory, waiting: " << model_factory.waiting() << " "
           << "created: " << model_factory.total_created() << " "
           << "empty: " << model_factory.total_empty() << " "
           << "total: " << model_factory.total() << endl;

    }

    glActiveTexture(GL_TEXTURE0);
    nvgFontBlur(mNVGContext, 0.8f);
    nvgFontSize(mNVGContext, 20.0f);
    nvgTextBox(mNVGContext, 10, 20, width - 10, os.str().c_str(), NULL);
}

int Konstructs::translate_button(int button) {
    switch (button) {
        case GLFW_MOUSE_BUTTON_1:
            return 1;
        case GLFW_MOUSE_BUTTON_2:
            return 2;
        case GLFW_MOUSE_BUTTON_3:
            return 3;
    }
}

bool Konstructs::update_view_distance() {
    double frame_fps = 1.15 / frame_time;
    float fps = settings.client.frames_per_second;

    if (frame_fps > 0.0 && frame_fps < fps && radius > 1) {
        view_distance = view_distance - (float) CHUNK_SIZE * 0.2f * ((fps - (float) frame_fps) / fps);
        return true;
    } else if (frame_fps >= fps
               && radius < settings.client.radius_max
               && model_factory.waiting() == 0
               && radius <= network.get_client()->get_loaded_radius()) {
        view_distance = view_distance + 0.05f;
        return true;
    } else {
        return false;
    }
}

void Konstructs::update_radius() {
    if (update_view_distance()) {
        int new_radius = (int) (view_distance / (float) CHUNK_SIZE) + 1;
        radius = new_radius;
        network.get_client()->set_radius(radius);
    }
}

void Konstructs::handle_mouse() {
    int exclusive =
            glfwGetInputMode(mGLFWWindow, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;
    if (exclusive && (px || py)) {
        double mx, my;
        glfwGetCursorPos(mGLFWWindow, &mx, &my);
        float m = 0.0025;
        float drx = (mx - px) * m;
        float dry = (my - py) * m;

        player.rotate_x(dry);
        player.rotate_y(drx);
        px = mx;
        py = my;

        if (click_delay == 0) {
            if (looking_at) {
                auto &l = *looking_at;
                uint8_t direction = direction_from_vector(l.first.position, l.second.position);
                uint8_t rotation = rotation_from_vector(direction, player.camera_direction());
                if (glfwGetMouseButton(mGLFWWindow, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
                    click_delay = MOUSE_CLICK_DELAY_IN_FRAMES;
                    network.get_client()->click_at(1, l.second.position, translate_button(GLFW_MOUSE_BUTTON_1), hud.get_selection(),
                                    direction, rotation);
                } else if (glfwGetMouseButton(mGLFWWindow, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS &&
                           player.can_place(l.first.position, world, blocks)) {
                    optional <ItemStack> selected = hud.selected();
                    if (selected) {
                        BlockData block = {selected->type, selected->health,
                                           DIRECTION_UP,
                                           ROTATION_IDENTITY
                        };
                        if (blocks.is_orientable[block.type]) {
                            block.direction = direction;
                            block.rotation = rotation;
                        }
                        auto chunk_opt =
                                world.chunk_by_block(l.first.position);
                        if (chunk_opt) {
                            ChunkData updated_chunk =
                                    chunk_opt->set(l.first.position, block);
                            world.insert(updated_chunk);
                            model_factory.create_models({updated_chunk.position}, world);
                        }
                    }
                    click_delay = MOUSE_CLICK_DELAY_IN_FRAMES;
                    network.get_client()->click_at(1, l.first.position, translate_button(GLFW_MOUSE_BUTTON_2), hud.get_selection(),
                                    direction, rotation);
                } else if (glfwGetMouseButton(mGLFWWindow, GLFW_MOUSE_BUTTON_3) == GLFW_PRESS) {
                    click_delay = MOUSE_CLICK_DELAY_IN_FRAMES;
                    network.get_client()->click_at(1, l.second.position, translate_button(GLFW_MOUSE_BUTTON_3), hud.get_selection(),
                                    direction, rotation);
                }
            } else if (glfwGetMouseButton(mGLFWWindow, GLFW_MOUSE_BUTTON_3) == GLFW_PRESS) {
                click_delay = MOUSE_CLICK_DELAY_IN_FRAMES;
                network.get_client()->click_at(0, Vector3i::Zero(), translate_button(GLFW_MOUSE_BUTTON_3), hud.get_selection(),
                                0, 0);
            }
        } else {
            click_delay--;
        }
    } else {
        glfwGetCursorPos(mGLFWWindow, &px, &py);
    }
}

void Konstructs::handle_keys() {
    int sx = 0;
    int sz = 0;
    bool jump = false;
    bool sneak = false;
    double now = glfwGetTime();
    double dt = now - last_frame;
    frame_time = now - last_frame;
    dt = MIN(dt, 0.2);
    dt = MAX(dt, 0.0);
    last_frame = now;
    if (glfwGetKey(mGLFWWindow, settings.keys.up)) {
        sz--;
    }
    if (glfwGetKey(mGLFWWindow, settings.keys.down)) {
        sz++;
    }
    if (glfwGetKey(mGLFWWindow, settings.keys.left)) {
        sx--;
    }
    if (glfwGetKey(mGLFWWindow, settings.keys.right)) {
        sx++;
    }
    if (glfwGetKey(mGLFWWindow, settings.keys.jump)) {
        jump = true;
    }
    if (glfwGetKey(mGLFWWindow, settings.keys.sneak)) {
        sneak = true;
    }
    network.get_client()->position(player.update_position(sz, sx, (float) dt, world,
                                           blocks, near_distance, jump, sneak),
                    player.rx(), player.ry());
    Vector3i new_chunk(chunked_vec(player.camera()));
    if (new_chunk != player_chunk) {
        player_chunk = new_chunk;
        network.get_client()->set_player_chunk(player_chunk);
    }
}

void Konstructs::close_hud() {
    hud.set_interactive(false);
    glfwSetInputMode(mGLFWWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    network.get_client()->close_inventory();
    for (int i = 0; i < 17; i++) {
        for (int j = 1; j < 14; j++) {
            Vector2i pos(i, j);
            hud.reset_background(pos);
            hud.reset_stack(pos);
        }
    }
}

float Konstructs::time_of_day() {
    if (day_length <= 0) {
        return 0.5;
    }
    float t;
    t = glfwGetTime();
    t = t / day_length;
    t = t - (int) t;
    return t;
}

float Konstructs::daylight() {
    float timer = time_of_day();
    if (timer < 0.5) {
        float t = (timer - 0.25) * 100;
        return 1 / (1 + powf(2, -t));
    } else {
        float t = (timer - 0.85) * 100;
        return 1 - 1 / (1 + powf(2, -t));
    }
}


void Konstructs::show_menu(int state, string message) {
    using namespace nanogui;

    glfwSetInputMode(mGLFWWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glActiveTexture(GL_TEXTURE0);

    FormHelper *gui = new FormHelper(this);
    window = gui->addWindow({0, 0}, "Main Menu");
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
                network.setup_connection(settings.server, mGLFWWindow);
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
            network.setup_connection(settings.server, mGLFWWindow);
            save_settings(settings);
        }
    });

    window->center();
    performLayout(mNVGContext);
    menu_state = true;
}