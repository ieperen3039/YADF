#version 130

in vec3 position;
in vec3 vertexNormal;

// normal of the vertex
out vec3 mVertexNormal;
// position of the vertex
out vec3 mVertexPosition;

uniform mat4 modelMatrix;
uniform mat4 viewProjectionMatrix;
uniform mat3 normalMatrix;

void main()
{
	vec4 mPosition = modelMatrix * vec4(position, 1.0);
    gl_Position = viewProjectionMatrix * mPosition;

	mVertexNormal = normalize(normalMatrix * vertexNormal);
    mVertexPosition = mPosition.xyz;
}
