#version 450 core

layout (location = 1) in vec2 in_uv;

layout (location = 0) out vec4 out_color;

layout(push_constant) uniform BoardBlock {
    vec4 lightColor;
    vec4 darkColor;
} colors;

void main() {
    float cells = 8.0;
    vec2 board = in_uv * cells;

    ivec2 cell = ivec2(floor(board));
    bool checker = ((cell.x + cell.y) % 2) == 1;

    out_color = checker ? colors.lightColor : colors.darkColor;
}