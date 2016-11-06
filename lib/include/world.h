#ifndef __WORLD_H__
#define __WORLD_H__

#include <unordered_map>
#include <memory>
#include "matrix.h"
#include "client.h"
#include "chunk.h"

namespace konstructs {
    using nonstd::optional;

    class World {
    public:
        int size() const;
        void delete_unused_chunks(const Vector3i player_chunk, const int radi);
        void insert(const ChunkData data);
        const BlockData get_block(const Vector3i &block_pos) const;
        const ChunkData chunk_at(const Vector3i &block_pos) const;
        const optional<ChunkData> chunk_opt(const Vector3i &chunk_pos) const;
        const std::vector<ChunkData> atAndAround(const Vector3i &pos) const;
        std::unordered_map<Vector3i, ChunkData, matrix_hash<Vector3i>>::const_iterator find(const Vector3i &pos) const;
        std::unordered_map<Vector3i, ChunkData, matrix_hash<Vector3i>>::const_iterator end() const;
    private:
        std::unordered_map<Vector3i, ChunkData, matrix_hash<Vector3i>> chunks;
    };
};

#endif
