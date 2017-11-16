#version 330 core

uniform sampler2D texture_diffuse1;

in vec2 TexCoords;
in vec3 normal;
in vec4 posW;
in vec4 posWV;
in vec4 posWVP;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outPos;

void main()
{    
	outColor = texture(texture_diffuse1, TexCoords);
	
	outNormal = vec4(normalize(normal)*0.5+vec3(0.5), 1);
	
	outPos = vec4(vec3(1.0-step(fract(posW.xyz),vec3(0.98)))+step(vec3(0.0),posW.xyz)*0.33, 1);
	//outPos = vec4(vec3(1.0-step(fract(posW.xyz),vec3(1.0+posWV.z*0.01))), 1);
	//outPos = vec4(posW.xyz,1);
	//outPos = vec4(vec3(-posWV.z)*0.01,1.0);
}

