#version 330 core
layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoords;

uniform mat4 modelMatrix;
uniform mat4 projMatrix;
uniform mat4 viewMatrix;

out vec3 normal;
out vec4 posW;
out vec4 posWV;
out vec4 posWVP;
out vec2 TexCoords;

void main()
{
	mat3 normalMatrix = transpose(inverse(mat3(viewMatrix * modelMatrix)));

	normal = vNormal.xyz;
    TexCoords = vTexCoords;
	
	posW = modelMatrix * vec4(vPosition, 1);
	posWV = viewMatrix * posW;
	posWVP = projMatrix * posWV;
    gl_Position = posWVP;
}

