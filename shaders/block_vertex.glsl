#version 120

uniform mat4 matrix;
uniform vec3 camera;
uniform float fog_distance;
uniform int ortho;
uniform vec4 chunk;
uniform mat4 rotation;
uniform float timer;

attribute vec4 position;
attribute vec3 normal;
attribute vec4 uv;

varying vec2 fragment_uv;
varying float fragment_ao;
varying float fragment_light;
varying float fog_factor;
varying float fog_height;
varying float diffuse;

const float pi = 3.14159265;
const vec3 light_direction = normalize(vec3(-1.0, 1.0, -1.0));

void main() {
    vec4 local_position = rotation * (position + chunk);

    // Water waves
    if (uv.s > 1 - 1.0/8.0 && uv.t < 1 - 1.0/8.0) {
        local_position.y = local_position.y
            + sin(0.5 * local_position.x + timer * 50)/4
            + sin(0.5 * local_position.z + timer * 40)/4
            + 0.5;
    }

    // Plants
    if (uv.t > 0.2 && uv.t < 0.3) {
        local_position.x = local_position.x + sin(0.1 * local_position.x + timer * 600)/16;
        local_position.z = local_position.z + sin(0.1 * local_position.z + timer * 800)/18;
    }

    gl_Position = matrix * local_position;
    fragment_uv = uv.xy;
    fragment_ao = 0.3 + (1.0 - uv.z) * 0.7;
    fragment_light = uv.w;
    diffuse = max(0.0, dot(normal, light_direction));
    if (bool(ortho)) {
        fog_factor = 0.0;
        fog_height = 0.0;
    }
    else {
        float camera_distance = distance(camera, vec3(local_position));
        fog_factor = pow(clamp(camera_distance / fog_distance, 0.0, 1.0), 4.0);
        float dy = local_position.y - camera.y;
        float dx = distance(local_position.xz, camera.xz);
        fog_height = (atan(dy, dx) + pi / 2) / pi;
    }
}
