#version 430

#define BLOCK_SIZE 17

layout (local_size_x = 1, local_size_y = 1) in;

layout(std430, binding = 3) buffer gridLayout
{
	float grid[];
};

float density(vec3 p)
{
	p /= (BLOCK_SIZE - 1);

	return min(-(length(p - vec3(.5)) - .3),length(p - vec3(.3)) - .3);
}

void main()
{
	ivec2 index = ivec2(gl_GlobalInvocationID.xy);
	
	for (int i=0;i<BLOCK_SIZE;++i) {
		vec3 p = vec3(index,i);

		float d = density(p);

		grid[(index.x*BLOCK_SIZE + index.y)*BLOCK_SIZE + i] = d;
	}
}