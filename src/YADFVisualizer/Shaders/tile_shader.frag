#version 130
#pragma optionNV(unroll all)

in vec2 vTextureCoords;
out vec4 frag_color;

const int MAX_NUM_MATERIALS = 8;

uniform sampler2D sprite;
uniform vec3 colors[MAX_NUM_MATERIALS];

void main() {
    vec4 color = texture(sprite, vTextureCoords);
    if (color.w == 0) discard;

    frag_color = vec4(color.rgb, 1.0);
//    frag_color = vec4(0, 0, 0, 1);
}  