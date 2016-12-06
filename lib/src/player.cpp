#include <algorithm>
#include "player.h"
#include "matrix.h"
#include "math.h"

namespace konstructs {

    using namespace Eigen;
    using nonstd::nullopt;

    static float CAMERA_OFFSET = 3.5f;
    static Vector3f CAMERA_OFFSET_VECTOR = Vector3f(0, CAMERA_OFFSET, 0);

    static bool block_is_obstacle(const optional<BlockData> &block, const BlockTypeInfo &blocks) {
        return block && blocks.is_obstacle[(*block).type];
    }

    Player::Player(const int id, const Vector3f position, const float rx,
                   const float ry):
        id(id), position(position), mrx(rx), mry(ry), flying(false), dy(0) {}

    Matrix4f Player::direction() const {
        return (Affine3f(AngleAxisf(mrx, Vector3f::UnitX())) *
                Affine3f(AngleAxisf(mry, Vector3f::UnitY()))).matrix();
    }

    Matrix4f Player::translation() const {
        return (Affine3f(Translation3f(position)) *
                Affine3f(AngleAxisf(-mry, Vector3f::UnitY())) *
                Affine3f(AngleAxisf(-mrx, Vector3f::UnitX()))).matrix();
    }

    Matrix4f Player::view() const {
        return (Affine3f(AngleAxisf(mrx, Vector3f::UnitX())) *
                Affine3f(AngleAxisf(mry, Vector3f::UnitY())) *
                Affine3f(Translation3f(-camera()))).matrix();
    }

    Vector3f Player::camera() const {
        return position + CAMERA_OFFSET_VECTOR;
    }

    Vector3f Player::camera_direction() const {
        float m = cosf(mrx);
        Vector3f vec(cosf(mry - (M_PI / 2.0f)) * m, -sinf(mrx), sinf(mry - (M_PI / 2.0f)) * m);
        vec.normalize();
        return vec;
    }

    Vector3i Player::feet() const {
        return Vector3i(roundf(position[0]), roundf(position[1]) - 1, roundf(position[2]));
    }

    bool Player::can_place(Vector3i block, const World &world, const BlockTypeInfo &blocks) {
        Vector3i f = feet();
        /* Are we trying to place blocks on ourselves? */
        if(block(0) == f(0) && block(2) == f(2) && block(1) >= f(1) && block(1) < f(1) + 2) {
            /* We may place on our feet under certain circumstances */
            if(f(1) == block(1)) {
                /* Allow placing on our feet if the block above our head is not an obstacle*/
                return !block_is_obstacle(world.get_block(Vector3i(f(0), f(1) + 2, f(2))), blocks);
            } else {
                /* We are never allowed to place on our head */
                return false;
            }
        }
        return true;
    }

    Vector3f Player::update_position(int sz, int sx, float dt,
                                     const World &world, const BlockTypeInfo &blocks,
                                     const float near_distance, const bool jump,
                                     const bool sneaking) {
        optional<ChunkData> chunk_opt = world.chunk_by_block(position);

        if(chunk_opt) { // Only update position if the chunk we are in is loaded
            float vx = 0, vy = 0, vz = 0;
            if (!sz && !sx) { // Not mowing in X or Z
                vx = 0;
                vz = 0;
            } else { // Moving in X or Z


                float strafe = atan2f(sz, sx);

                if (flying) {
                    float m = cosf(mrx);
                    float y = sinf(mrx);
                    if (sx) {
                        if (!sz) {
                            y = 0;
                        }
                        m = 1;
                    }
                    if (sz < 0) {
                        y = -y;
                    }
                    vx = cosf(mry + strafe) * m;
                    vy = y;
                    vz = sinf(mry + strafe) * m;
                } else {
                    vx = cosf(mry + strafe);
                    vy = 0;
                    vz = sinf(mry + strafe);
                }
            }

            if(jump) {
                if(flying) {
                    // Jump in flight moves upward at constant speed
                    vy = 1;
                } else if(dy == 0) {
                    // Jump when walking changes the acceleration upwards to 8
                    dy = 6.0f *  CAMERA_OFFSET;
                } else {
                    // Get middle of block
                    Vector3i iPos((int)(position[0] + 0.5f), (int)(position[1]), (int)(position[2] + 0.5f));
                    auto chunk = world.chunk_by_block(iPos);

                    if(chunk && blocks.state[chunk->get(iPos).type] == STATE_LIQUID) {
                        dy = 5.5;
                    }
                }
            }

            float speed = flying ? 20 : 5;
            int estimate =
                roundf(sqrtf(powf(vx * speed, 2) +
                             powf(vy * speed + std::abs(dy) * 2, 2) +
                             powf(vz * speed, 2)) * dt * 8);
            int step = std::max(8, estimate);
            float ut = dt / step;
            vx = vx * ut * speed;
            vy = vy * ut * speed;
            vz = vz * ut * speed;
            for (int i = 0; i < step; i++) {
                if (flying) {
                    // When flying upwards acceleration is constant i.e. not falling
                    dy = 0;
                } else {
                    // Calculate "gravity" by decreasing upwards acceleration
                    dy -= ut * 25;
                    dy = std::max(dy, -250.0f);
                }
                position += Vector3f(vx, vy + dy * ut, vz);
                if (collide(world, blocks, near_distance, sneaking)) {
                    dy = 0;
                }
            }
            if (position[1] < 0) {
                position[1] = 2;
            }
        }
        return position;
    }

    optional<pair<Block, Block>> Player::looking_at(const World &world,
    const BlockTypeInfo &blocks) const {
        optional<pair<Block, Block>> block(nullopt);
        float best = 0;
        const Vector3f v = camera_direction();
        const Vector3f camera_position = camera();
        int p = chunked(camera_position[0]);
        int q = chunked(camera_position[2]);
        int k = chunked(camera_position[1]);
        const auto &atAndAround = world.atAndAround({p, q, k});
        for (const auto &chunk: atAndAround) {
            const auto &seen = chunk.get(camera_position, v, 8.0f, blocks);
            if (seen) {
                auto h = seen->second;
                float d = sqrtf(powf(h.position[0] - camera_position[0], 2) +
                                powf(h.position[1] - camera_position[1], 2) +
                                powf(h.position[2] - camera_position[2], 2));
                if (best == 0 || d < best) {
                    best = d;
                    block = seen;
                }
            }
        }
        return block;
    }

    void Player::rotate_x(float speed) {
        mrx += speed;
        mrx = std::max(mrx, -((float)M_PI / 2.0f));
        mrx = std::min(mrx, ((float)M_PI / 2.0f));
    }

    void Player::rotate_y(float speed) {
        mry += speed;
        if (mry < 0) {
            mry += (M_PI * 2);
        }
        if (mry >= (M_PI * 2)) {
            mry -= (M_PI * 2);
        }
    }

    void Player::fly() {
        flying = !flying;
    }

    float Player::rx() {
        return mrx;
    }

    float Player::ry() {
        return mry;
    }

    int Player::collide(const World &world, const BlockTypeInfo &blocks,
                        const float near_distance, const bool sneaking) {
        int result = 0;
        float x = position[0];
        float y = position[1];
        float z = position[2];
        int height = 2;
        int p = chunked(x);
        int q = chunked(z);
        int k = chunked(y);
        int nx = roundf(x);
        int ny = roundf(y);
        int nz = roundf(z);
        float px = x - nx;
        float py = y - ny;
        float pz = z - nz;
        float pad = near_distance * 2;

        try {

            if (block_is_obstacle(world.get_block(feet()), blocks)) {
                position[1] += 1.0f;
                return 1;
            }

            if(sneaking) {
                if (px < -pad && !block_is_obstacle(world.get_block(Vector3i(nx - 1, ny - 2, nz)), blocks)) {
                    position[0] = nx - pad;
                }
                if (px > pad && !block_is_obstacle(world.get_block(Vector3i(nx + 1, ny - 2, nz)), blocks)) {
                    position[0] = nx + pad;
                }
                if (pz < -pad && !block_is_obstacle(world.get_block(Vector3i(nx, ny - 2, nz - 1)), blocks)) {
                    position[2] = nz - pad;
                }
                if (pz > pad && !block_is_obstacle(world.get_block(Vector3i(nx, ny - 2, nz + 1)), blocks)) {
                    position[2] = nz + pad;
                }
            }
            for (int dy = 0; dy < height; dy++) {
                if (px < -pad && block_is_obstacle(world.get_block(Vector3i(nx - 1, ny - dy, nz)), blocks)) {
                    position[0] = nx - pad;
                }
                if (px > pad && block_is_obstacle(world.get_block(Vector3i(nx + 1, ny - dy, nz)), blocks)) {
                    position[0] = nx + pad;
                }
                if (py < -pad && block_is_obstacle(world.get_block(Vector3i(nx, ny - dy - 1, nz)), blocks)) {
                    position[1] = ny - pad;
                    result = 1;
                }
                if (py > (pad - CAMERA_OFFSET) && block_is_obstacle(world.get_block(Vector3i(nx, ny - dy + 1, nz)), blocks)) {
                    position[1] = ny + pad - CAMERA_OFFSET;
                    result = 1;
                }
                if (pz < -pad && block_is_obstacle(world.get_block(Vector3i(nx, ny - dy, nz - 1)), blocks)) {
                    position[2] = nz - pad;
                }
                if (pz > pad && block_is_obstacle(world.get_block(Vector3i(nx, ny - dy, nz + 1)), blocks)) {
                    position[2] = nz + pad;
                }
            }
        } catch(std::out_of_range e) {
            /* chunk was not loaded yet */
            return result;
        }
        return result;
    }
};
