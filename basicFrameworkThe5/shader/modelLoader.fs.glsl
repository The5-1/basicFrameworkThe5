#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_albedo;
uniform sampler2D texture_normal;

layout (location = 0) out vec4 albedo;
layout (location = 1) out vec3 normal;

void main()
{    
    albedo = texture(texture_albedo, TexCoords);
    normal = texture(texture_normal, TexCoords).xyz;

}

