#include "world.h"

namespace konstructs {

    int World::size() const {
        return chunks.size();
    }

    void World::delete_unused_chunks(const Vector3i player_chunk, const int radi) {
        for ( auto it = chunks.begin(); it != chunks.end();) {
            if ((it->second->position - player_chunk).norm() > radi) {
                it = chunks.erase(it);
            } else {
                ++it;
            }
        }
    }

    void World::insert(std::shared_ptr<ChunkData> data) {
        /* Overwrite any existing chunk, we always want the latest data */
        const Vector3i pos = data->position;
        chunks.erase(pos);
        chunks.insert({pos, data});
    }

    const BlockData World::get_block(const Vector3i &block_pos) const
    {
        auto iter=chunks.find(chunked_vec_int(block_pos));

        if(iter!=chunks.end())
            return iter->second->get(block_pos);
        return{0, 0};

    }

    const std::shared_ptr<ChunkData> World::chunk_at(const Vector3i &block_pos) const {
        return chunk(chunked_vec_int(block_pos));
    }

    const std::shared_ptr<ChunkData> World::chunk(const Vector3i &chunk_pos) const
    {
        auto iter=chunks.find(chunk_pos);

        if(iter != chunks.end())
            return iter->second;
        return std::shared_ptr<ChunkData>();
    }

    const std::vector<std::shared_ptr<ChunkData>> World::atAndAround(const Vector3i &pos) const 
    {
        std::vector<std::shared_ptr<ChunkData>> result;
        for(int i = -1; i <= 1; i++) 
        {
            for(int j = -1; j <= 1; j++) 
            {
                for(int k = -1; k <= 1; k++) 
                {
                    try 
                    {
                        auto iter=chunks.find(pos+Vector3i(i, j, k));

                        if(iter != chunks.end())
                            result.push_back(iter->second);

                    } catch(std::out_of_range e) {
                        // Not in this chunk
                    }
                }
            }
        }
        return result;
    }

    std::unordered_map<Vector3i, std::shared_ptr<ChunkData>, matrix_hash<Vector3i>>::const_iterator World::find(const Vector3i &pos) const {
        return chunks.find(pos);
    }

    std::unordered_map<Vector3i, std::shared_ptr<ChunkData>, matrix_hash<Vector3i>>::const_iterator World::end() const {
        return chunks.end();
    }

};
