#version 330
layout(location = 0)  out vec4 out0; // color 

uniform sampler2D diffuseTex;
uniform sampler2D normalTex;
uniform sampler2D positionTex;
uniform sampler2D depthTex;

uniform mat4 projMatrix;

uniform vec3 cameraDirection;
uniform vec3 cameraPosition;

in vec2 texCoords;

#define LIGHTPOS vec3(0.0,30.0,0.0)


void main()
{
	//Variables
	vec4 map_albedo = texture2D(diffuseTex, texCoords);
	float depth = texture2D(depthTex, texCoords).x;
	vec3 position =  texture2D(positionTex, texCoords).xyz;
	vec3 normal = texture2D( normalTex, texCoords).xyz;
	vec3 fragmentvec = texture2D(normalTex, texCoords).xyz;

	vec3 diffuse = map_albedo.rgb*dot(normal,LIGHTPOS - position) / length(LIGHTPOS - position);


	//float cameraDistance = distance(position, cameraPosition);

	vec3 positionvec = cameraPosition * -2.0;
	vec3 viewvec = cameraDirection;

	vec3 fragmentunitvec = normalize(fragmentvec);
	vec3 viewunitvec = normalize(viewvec);

	float p34 = projMatrix[2][3];
	float p33 = projMatrix[2][2];
	float z = -p34 / (depth + p33);
	float u = z / dot(fragmentunitvec, viewunitvec);
	vec3 worldfragment = positionvec + (u * fragmentunitvec);

	//Position from texture
	worldfragment = position;

	vec4 fragmentcolor = vec4(1.0,1.0,1.0,0.0);
	
	float integral = 0.02 * -(exp(-positionvec.y*0.01)-exp(-worldfragment.y*0.01));
	
	float F = u * integral / (positionvec.y - worldfragment.y);

	float alpha = 1.0 - exp(-F);

	//out0 = vec4(alpha * map_albedo.xyz + (1.0 - alpha) * fragmentcolor.xyz, 1.0f);
	out0 = vec4((1.0 - alpha) * map_albedo.xyz + alpha * fragmentcolor.xyz, 1.0f);

	//out0 = vec4(position, 1.0);

	//out0 = fragmentcolor;
	//out0 = vec4(vec3((0.1-position.y)*10.0),1.0);
	out0 = vec4(diffuse,1.0);
}