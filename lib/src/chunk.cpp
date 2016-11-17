#include <iostream>
#include <memory>
#include <string.h>
#include "block.h"
#include "compress.h"
#include "chunk.h"
#include "matrix.h"

namespace konstructs {
    using nonstd::nullopt;

    ChunkData SOLID_CHUNK(SOLID_TYPE);
    ChunkData VACUUM_CHUNK(VACUUM_TYPE);


    std::shared_ptr<BlockData> read_chunk_data(uint8_t *buffer,
            std::unordered_map<uint16_t, std::shared_ptr<BlockData>> &cached_data) {
        BlockData *blocks = new BlockData[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE];
        uint16_t chunk_type = buffer[0] + (buffer[1] << 8);
        bool use_cached = true;
        for(int i = 0; i < CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE; i++) {
            blocks[i].type = buffer[i * BLOCK_SIZE] + (buffer[i * BLOCK_SIZE + 1] << 8);
            blocks[i].health = buffer[i * BLOCK_SIZE + 2] + ((buffer[i * BLOCK_SIZE + 3] & 0x07) << 8);
            blocks[i].direction = (buffer[i * BLOCK_SIZE + 3] & 0xE0) >> 5;
            blocks[i].rotation = (buffer[i * BLOCK_SIZE + 3] & 0x18) >> 3;
            blocks[i].ambient = (buffer[i * BLOCK_SIZE + 4] & 0xF);
            blocks[i].r = (buffer[i * BLOCK_SIZE + 4] & 0xF0) >> 4;
            blocks[i].g = (buffer[i * BLOCK_SIZE + 5] & 0xF);
            blocks[i].b = (buffer[i * BLOCK_SIZE + 5] & 0xF0) >> 4;
            blocks[i].light = (buffer[i * BLOCK_SIZE + 6] & 0xF);
            if(blocks[i].type != chunk_type || blocks[i].light > 0 || blocks[i].ambient < AMBIENT_LIGHT_FULL) {
                use_cached = false;
            }
        }
        if(use_cached) {
            try {
                auto r = cached_data.at(chunk_type);
                delete[] blocks;
                return r;
            } catch(std::out_of_range e)  {
                std::shared_ptr<BlockData> r(blocks, std::default_delete<BlockData[]>());
                cached_data.insert({chunk_type, r});
                return r;
            }
        } else {
            std::shared_ptr<BlockData> r(blocks, std::default_delete<BlockData[]>());
            return r;
        }
    }

    int chunked_int(int p) {
        if(p < 0) {
            return (p - CHUNK_SIZE + 1) / CHUNK_SIZE;
        } else {
            return p / CHUNK_SIZE;
        }
    }

    int chunked(float p) {
        return chunked_int(roundf(p));
    }

    Vector3i chunked_vec_int(const Vector3i position) {
        return Vector3i(chunked_int(position[0]), chunked_int(position[2]), chunked_int(position[1]));
    }

    Vector3i chunked_vec(const Vector3f position) {
        return chunked_vec_int(position.cast<int>());
    }

    ChunkData::ChunkData(const Vector3i position, char *compressed, const int size, uint8_t *buffer,
                         std::unordered_map<uint16_t, std::shared_ptr<BlockData>> &cached_data):
        position(position) {
        int out_size = inflate_data(compressed + BLOCKS_HEADER_SIZE,
                                    size - BLOCKS_HEADER_SIZE,
                                    (char*)buffer, BLOCK_BUFFER_SIZE);
        revision =
            compressed[2] +
            (compressed[2 + 1] << 8) +
            (compressed[2 + 2] << 16) +
            (compressed[2 + 3] << 24);
        blocks = read_chunk_data(buffer, cached_data);
    }

    ChunkData::ChunkData(const uint16_t type) : revision(0) {
        BlockData *b = new BlockData[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE];
        for(int i = 0; i < CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE; i++) {
            b[i].type = type;
            b[i].health = MAX_HEALTH;
            b[i].direction = DIRECTION_UP;
            b[i].rotation = ROTATION_IDENTITY;
            b[i].ambient = AMBIENT_LIGHT_DARK;
            b[i].r = 0;
            b[i].g = 0;
            b[i].b = 0;
            b[i].light = 0;
        }
        blocks = std::shared_ptr<BlockData>(b, std::default_delete<BlockData[]>());
    }

    ChunkData::ChunkData(const Vector3i position, const uint32_t revision, BlockData *b) :
        position(position), revision(revision) {
        blocks = std::shared_ptr<BlockData>(b, std::default_delete<BlockData[]>());
    }

    BlockData ChunkData::get(const Vector3i &pos) const {
        int lx = pos[0] - position[0] * CHUNK_SIZE;
        int ly = pos[1] - position[2] * CHUNK_SIZE;
        int lz = pos[2] - position[1] * CHUNK_SIZE;

        // TODO: Looking for a block in the wrong chunk is a bit weird, but hit code does it
        if(lx < CHUNK_SIZE && ly < CHUNK_SIZE && lz < CHUNK_SIZE &&
                lx >= 0 && ly >= 0 && lz >= 0) {
            int i = lx+ly*CHUNK_SIZE+lz*CHUNK_SIZE*CHUNK_SIZE;
            return blocks.get()[i];
        } else {
            return {0, 0};
        }
    }

    ChunkData ChunkData::set(const Vector3i &pos, const BlockData &data) const {
        int lx = pos[0] - position[0] * CHUNK_SIZE;
        int ly = pos[1] - position[2] * CHUNK_SIZE;
        int lz = pos[2] - position[1] * CHUNK_SIZE;

        BlockData *new_blocks = new BlockData[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE];
        BlockData *b = blocks.get();
        memcpy(new_blocks, b, CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE*sizeof(BlockData));

        new_blocks[lx+ly*CHUNK_SIZE+lz*CHUNK_SIZE*CHUNK_SIZE] = data;

        // A chunk that we altered ourselves is treated as invalid

        return ChunkData(position, 0, new_blocks);
    }

    /**
     * Using a camera position and a camera direction, find the
     * closest within max_distance that intersect the directional
     * vector.
     */
    optional<pair<Block, Block>> ChunkData::get(const Vector3f &camera_position,
                              const Vector3f &camera_direction,
                              const float max_distance,
    const BlockTypeInfo &blocks) const {
        int m = 4;
        Vector3f pos = camera_position;
        Vector3i blockPos(roundf(pos[0]), roundf(pos[1]), roundf(pos[2]));
        for (int i = 0; i < max_distance * m; i++) {
            const Vector3i nBlockPos(roundf(pos[0]), roundf(pos[1]), roundf(pos[2]));
            if (nBlockPos != blockPos) {
                BlockData data = get(nBlockPos);
                if (blocks.is_obstacle[data.type] || blocks.is_plant[data.type]) {
                    return optional<pair<Block, Block>>(pair<Block, Block>(Block(blockPos, data),
                                                        Block(nBlockPos, data)));
                }
                blockPos = nBlockPos;
            }
            pos += (camera_direction / m);
        }
        return nullopt;
    }
};
