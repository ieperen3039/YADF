#version 330
#pragma optionNV(unroll all)

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in int materialBit;

// normal of the vertex
out vec3 mVertexNormal;
// position of the vertex
out vec3 mVertexPosition;
// material of the triangle
flat out int mMaterialBit;

const int MAX_MATERIALS = 16;

uniform mat4 modelMatrix;
uniform mat4 viewProjectionMatrix;
uniform mat3 normalMatrix;

void main() {
    vec4 mPosition = modelMatrix * vec4(position, 1.0);
    gl_Position = viewProjectionMatrix * mPosition;

    mVertexNormal = normalize(normalMatrix * vertexNormal);
    mVertexPosition = mPosition.xyz;
    mMaterialBit = materialBit;
}