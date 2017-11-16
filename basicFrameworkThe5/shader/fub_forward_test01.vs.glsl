#version 330
layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoords;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform vec3 cameraPosition;

uniform vec3 lightDir;

uniform float uTime;

out vec2 TexCoords;
out vec3 NormalW;
out vec3 LightW;
out vec3 PosW;
out vec3 cameraPos;
out float depth;


void main() {
	mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
	
	TexCoords = vTexCoords;

	NormalW = normalMatrix * vNormal;
	LightW = lightDir;
	PosW = (modelMatrix * vec4(vPosition, 1.0)).xyz;

	cameraPos = cameraPosition;
	
	depth = -(viewMatrix * modelMatrix * vec4(vPosition, 1.0)).z;

	gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(vPosition, 1.0);
}
