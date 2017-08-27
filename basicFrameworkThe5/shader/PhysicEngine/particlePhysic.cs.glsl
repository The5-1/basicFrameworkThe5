#version 430 core

layout (local_size_x = 128, local_size_y = 1, local_size_z = 1) in;

layout (std430, binding = 0) buffer PositionBuffer {
	vec3 positions[];
};

void main(void){

	uint index = gl_GlobalInvocationID.x;

	//vec3 pos = positions[index];

	positions[index] -= vec3(0.0, 0.1, 0.0);

}