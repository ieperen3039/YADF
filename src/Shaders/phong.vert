#version 330
#pragma optionNV(unroll all)

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in int materialBit;

// normal of the vertex
out vec3 mVertexNormal;
// position of the vertex
out vec3 mVertexPosition;
// material of the triangle
flat out int mMaterialBit;

uniform vec3 origin;
uniform int rotation;
uniform mat4 viewProjectionMatrix;

vec3 rotateQuarters(vec3 vector, int quarters) {
    for (int i = 0; i < quarters; i++) {
        float x = vector.x;
        vector.x = -vector.y;
        vector.y = x;
    }
    return vector;
}

void main() {
    mVertexPosition = rotateQuarters(vertexPosition, rotation);
    gl_Position = viewProjectionMatrix * vec4(mVertexPosition, 1.0);

    mVertexNormal = rotateQuarters(vertexNormal, rotation);

    mMaterialBit = materialBit;
}