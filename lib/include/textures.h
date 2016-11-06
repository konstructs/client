#ifndef __TEXTURES_H__
#define __TEXTURES_H__

#include "tiny_obj_loader.h"

namespace konstructs {
#define BLOCK_TEXTURES 5
#define SKY_TEXTURE 2
#define FONT_TEXTURE 3
#define INVENTORY_TEXTURE 4
#define PLAYER_TEXTURE 6
#define DAMAGE_TEXTURE 7
#define HEALTH_BAR_TEXTURE 8
    void load_textures();
    tinyobj::shape_t load_player();
    std::string load_chunk_vertex_shader();
    std::string load_chunk_fragment_shader();
};
#endif
