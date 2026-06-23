#version 450 core

layout (location = 1) in vec2 in_uv;

layout (location = 0) out vec4 out_color;

void main() {
    float cells = 8.0;
    vec2 board = in_uv * cells;

    ivec2 cell = ivec2(floor(board));
    bool checker = ((cell.x + cell.y) % 2) == 1;

    vec3 light = vec3(0.93, 0.85, 0.72);
    vec3 dark  = vec3(0.25, 0.18, 0.12);
    vec3 color = checker ? light : dark;

	out_color = vec4(color, 1.0);
}