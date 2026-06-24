#version 450 core

layout (set = 1, binding = 1) uniform sampler2D tex;


layout (location = 0) in vec4 in_tint;
layout (location = 1) in vec2 in_uv;

layout (location = 0) out vec4 out_color;

void main() {
    float outline_thickness = 0.06f;

    float edge_dist = min(min(in_uv.x, 1.0 - in_uv.x), min(in_uv.y, 1.0 - in_uv.y));

    if (edge_dist < outline_thickness) {
        out_color = in_tint;
    } else {
        discard;
    }
}