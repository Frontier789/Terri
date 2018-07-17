#version 430

#define BLOCK_SIZE 5

layout (local_size_x = 1, local_size_y = 1) in;

layout(std430, binding = 3) buffer gridLayout
{
	float grid[BLOCK_SIZE][BLOCK_SIZE][BLOCK_SIZE];
};

float density(vec3 p)
{
	p /= (BLOCK_SIZE - 1);

	return length(p - vec3(.5)) - .3 + sin(p.x*7)*.05 + sin(p.y*5 + 7)*.05 + sin(p.z*4 + 2)*.1;
}

void main()
{
	ivec2 index = ivec2(gl_GlobalInvocationID.xy);
	
	for (int i=0;i<BLOCK_SIZE;++i) {
		vec3 p = vec3(index,i);

		float d = density(p);

		grid[index.x][index.y][i] = d;
	}
}