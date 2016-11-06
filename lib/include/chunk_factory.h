#ifndef __CHUNK_FACTORY_H__
#define __CHUNK_FACTORY_H__

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <condition_variable>

#include "chunk.h"
#include "world.h"
#include "matrix.h"

namespace konstructs {
    using std::shared_ptr;
    struct ChunkModelData {
        const Vector3i position;
        const ChunkData below;
        const ChunkData above;
        const ChunkData left;
        const ChunkData right;
        const ChunkData front;
        const ChunkData back;
        const ChunkData above_left;
        const ChunkData above_right;
        const ChunkData above_front;
        const ChunkData above_back;
        const ChunkData above_left_front;
        const ChunkData above_right_front;
        const ChunkData above_left_back;
        const ChunkData above_right_back;
        const ChunkData left_front;
        const ChunkData left_back;
        const ChunkData right_front;
        const ChunkData right_back;
        const ChunkData self;
    };

    class ChunkModelResult {
    public:
        ChunkModelResult(const Vector3i _position, const int components,
                         const int _faces);
        ~ChunkModelResult();
        const Vector3i position;
        const int size;
        const int faces;
        GLuint *data();
    private:
        GLuint *mData;
    };

    class ChunkModelFactory {
    public:
        ChunkModelFactory(const BlockTypeInfo &_block_data);
        int waiting();
        int total();
        int total_empty();
        int total_created();
        void update_player_chunk(const Vector3i &chunk);
        void create_models(const std::vector<Vector3i> &positions,
                           const World &world);
        std::vector<std::shared_ptr<ChunkModelResult>> fetch_models();
    private:
        int processed;
        int empty;
        int created;
        void worker();
        std::mutex mutex;
        std::condition_variable chunks_condition;
        Vector3i player_chunk;
        std::unordered_set<Vector3i, matrix_hash<Vector3i>> chunks;
        std::unordered_map<Vector3i, ChunkModelData, matrix_hash<Vector3i>> model_data;
        std::vector<std::shared_ptr<ChunkModelResult>> models;
        const BlockTypeInfo &block_data;
    };

    std::vector<ChunkModelData> adjacent(const Vector3i position, const World &world);
    const ChunkData get_chunk(const Vector3i &position,
                              const World &world);
    const ChunkModelData create_model_data(const Vector3i &position,
                                           const World &world);

    shared_ptr<ChunkModelResult> compute_chunk(const ChunkModelData &data, const BlockTypeInfo &block_data);
};
#endif
