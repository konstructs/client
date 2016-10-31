#version 330

/* Look-up tables for reconstructing cube vertices and normals.
 * They are all allocated as uniforms since for arrays this is faster
 * by several orders of a magnitude (otherwise they consume the per
 * execution unit memory of the shader).
 */

/* distance from the middle of the voxel */
const float N = 0.5;

/* This table contains all required vertexes for the different voxels.
 */
uniform vec3 positions[16] = vec3[16](
    vec3(-N, -N, -N), //0 - Block corners
    vec3(-N, -N, +N), //1
    vec3(-N, +N, -N), //2
    vec3(+N, -N, -N), //3
    vec3(+N, +N, -N), //4
    vec3(-N, +N, +N), //5
    vec3(+N, -N, +N), //6
    vec3(+N, +N, +N), //7
    vec3(+0, -N, -N), //8 - Plant corners
    vec3(+0, -N, +N), //9
    vec3(+0, +N, -N), //10
    vec3(+0, +N, +N), //11
    vec3(-N, -N, +0), //12
    vec3(-N, +N, +0), //13
    vec3(+N, -N, +0), //14
    vec3(+N, +N, +0)  //15
);

/*
 * This table contain normal vectors (the vector that points
 * perpendicular to the the plane formed by the triangle).
 * There are 6 normals, one for direction and axis.
 */
uniform vec3 normals[6] = vec3[6](
    vec3(-1, 0, 0), //0
    vec3(+1, 0, 0), //1
    vec3(0, +1, 0), //2
    vec3(0, -1, 0), //3
    vec3(0, 0, -1), //4
    vec3(0, 0, +1)  //5
);

/* Data offsets and mask */

/* x component */

/* First is the normal index (0 - 5) encoded in 3 bits */
const uint OFF_NORMAL = uint(0);
const uint MASK_NORMAL = uint(0x07);

/* Second is the vertex index (0 - 15) encoded in 4 bits */
const uint OFF_VERTEX = uint(3);
const uint MASK_VERTEX = uint(0x0F);

/* Then comes the x,y,z position of the block in the chunk,
 * encoded in 5 bits each */
const uint OFF_X = uint(7);
const uint OFF_Y = uint(12);
const uint OFF_Z = uint(17);
const uint MASK_POS = uint(0x1F);

/* Then the ambient occlusion (0 - 31) encoded in 5 bits. */
const uint OFF_AO = uint(22);
const uint MASK_AO = uint(0x1F);

/* And lastly the uv coordinates for the block damage, (0 - 8) and (0 - 1) encoded in 4 + 1 bits */
const uint OFF_DAMAGE_U = uint(27);
const uint MASK_DAMAGE_U = uint(0x0F);

const uint OFF_DAMAGE_V = uint(31);
const uint MASK_DAMAGE_V = uint(0x01);

/* y component */

/* First comes the UV coordinates for the texture of this face,
 * encoded in 5 bits each.
 */
const uint OFF_DU = uint(0);
const uint OFF_DV = uint(5);
const uint MASK_UV = uint(0x1F);

/* Second comes the ambient light level of the block encoded
 * in 4 bits
 */
const uint OFF_AL = uint(10);
const uint MASK_AL = uint(0x0F);

/* Third comes the light color and light level encoded with 4 bits
 * per RGB channel and 4 bits of strength
 */
const uint OFF_R = uint(14);
const uint MASK_R = uint(0x0F);

const uint OFF_G = uint(18);
const uint MASK_G = uint(0x0F);

const uint OFF_B = uint(22);
const uint MASK_B = uint(0x0F);

const uint OFF_LIGHT = uint(26);
const uint MASK_LIGHT = uint(0x0F);

/* UV stepping */
const float S = (1.0 / 16.0);
const float DS = (1.0 / 8.0);

/* Influences how much the damage is mixed into the block */
const float damage_weight = 0.6;

/* Projection and player translation */
uniform mat4 matrix;

/* Camera position */
uniform vec3 camera;

/* Fog distance */
uniform float fog_distance;

/* Chunk translation */
uniform mat4 translation;

/* The per vertex data as described above */
in uvec2 data;

/* Output to fragment shader */

/* UV coordinates in texture space */
out vec2 fragment_uv;
out vec2 damage_uv;

/* Damage */
flat out float damage_factor;

/* The ambient value */
out float ambient;
out float fragment_ao;



/* The light value */
out vec3 light;

out float fog_factor;
out float fog_height;

/* Diffuse lightning a.k.a. the sun */
out float diffuse;

const float PI = 3.14159265;
const vec3 light_direction = normalize(vec3(-1.0, 1.0, -1.0));

void main() {

    /* Extract data from x component */
    uint d1 = data.x;

    /* Extract the block face index (0 - 5) */
    uint normal = (d1 >> OFF_NORMAL) & MASK_NORMAL;

    /* Extract the corner of the face (0 - 3) */
    uint vertex = (d1 >> OFF_VERTEX) & MASK_VERTEX;

    /* Extract the amount of ambient occlusion */
    uint ao = (d1 >> OFF_AO) & MASK_AO;

    /* Extract block damage UV */
    uint damage_u = (d1 >> OFF_DAMAGE_U) & MASK_DAMAGE_U;
    uint damage_v = (d1 >> OFF_DAMAGE_V) & MASK_DAMAGE_V;

    /* Extract block position */
    uint x = (d1 >> OFF_X) & MASK_POS;
    uint y = (d1 >> OFF_Y) & MASK_POS;
    uint z = (d1 >> OFF_Z) & MASK_POS;

    /* Extract data from y component */
    uint d2 = data.y;

    /* Extract the block type texture index */
    uint du = (d2 >> OFF_DU) & MASK_UV;
    uint dv = (d2 >> OFF_DV) & MASK_UV;

    /* Extract the ambient light */
    uint al = (d2 >> OFF_AL) & MASK_AL;

    /* Extract light */
    uint r = (d2 >> OFF_R) & MASK_R;
    uint g = (d2 >> OFF_G) & MASK_G;
    uint b = (d2 >> OFF_B) & MASK_B;
    uint light_level = (d2 >> OFF_LIGHT) & MASK_LIGHT;

    /* All values extracted, shader code starts here */

    /* Create a translation matrix from the block position */
    mat4 block_translation = mat4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        x, y, z, 1);

    /* Calculate the vertex position within the chunk by applying the block translation */
    vec4 position = block_translation * vec4(positions[vertex], 1);

    /* Calculate the global position of the vertex by applying the chunk translation */
    vec4 global_position = translation * position;

    /* Apply projection */
    gl_Position = matrix * global_position;

    /* Calculate light */
    float rf = float(r) * 0.0625;
    float gf = float(g) * 0.0625;
    float bf = float(b) * 0.0625;
    float lf = float(light_level) * 0.0625;

    light = vec3(lf * rf, lf * gf, lf * bf);

    /* Calculate the ambient light */
    ambient = float(al + uint(1)) * 0.0625;

    /* Calculate ambient occlusion */
    fragment_ao = (1.0 - float(ao) * 0.03125 * 0.7);

    /* Calculate UV coordinates */
    fragment_uv = vec2(du * S, dv * S);
    damage_uv = vec2(damage_u * DS, damage_v);

    damage_factor = (damage_u * DS) * damage_weight;

    diffuse = clamp(dot(normals[normal], light_direction), 0.0, 1.0);

    float camera_distance = distance(camera, vec3(global_position));
    fog_factor = pow(clamp(camera_distance / fog_distance, 0.0, 1.0), 4.0);
    float dy = global_position.y - camera.y;
    float dx = distance(global_position.xz, camera.xz);
    fog_height = (atan(dy, dx) + PI / 2) / PI;
}
