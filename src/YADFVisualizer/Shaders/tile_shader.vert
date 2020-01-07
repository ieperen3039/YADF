#version 130
#pragma optionNV(unroll all)

in vec4 vertex;

out vec2 vTextureCoords;

uniform vec2 sprite_origin;
uniform vec2 camera_position;
uniform vec2 scaling;

void main() {
    vTextureCoords = vertex.zw;

    vec2 vPos = (sprite_origin + vertex.xy - camera_position) * scaling;
    gl_Position = vec4(vPos, 0.0, 1.0);
}