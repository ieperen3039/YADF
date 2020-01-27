#version 130
#pragma optionNV(unroll all)

in vec2 vTextureCoords;

out vec4 frag_color;

const int MAX_NUM_MATERIALS = 8;

uniform sampler2D sprite;
uniform vec3 colors[MAX_NUM_MATERIALS];
uniform float brightness; // [0 ... 1] with 0 is shadow and 1 is the original color

void main() {
    vec4 color = texture(sprite, vTextureCoords);
    if (color.w == 0) discard;

    color *= brightness;
    frag_color = vec4(color.rgb, 1.0);
}  