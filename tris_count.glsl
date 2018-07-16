#version 430

#define BLOCK_SIZE 5

layout (local_size_x = 1, local_size_y = 1) in;

layout(std430, binding = 3) buffer gridLayout
{
	float grid[BLOCK_SIZE][BLOCK_SIZE][BLOCK_SIZE];
};

layout(std430, binding = 4) buffer trinLayout
{
	int trin[BLOCK_SIZE-1][BLOCK_SIZE-1][BLOCK_SIZE-1];
};

int process_tetrahedron(ivec3 base,ivec3 d1,ivec3 d2)
{
	int n = int(grid[base.x][base.y][base.z] < 0) + 
			int(grid[base.x+d1.x][base.y+d1.y][base.z+d1.z] < 0) + 
			int(grid[base.x+d2.x][base.y+d2.y][base.z+d2.z] < 0) + 
			int(grid[base.x+1][base.y+1][base.z+1] < 0);

	return 2-abs(n-2);
}

void main()
{
	ivec2 index = ivec2(gl_GlobalInvocationID.xy);
	
	for (int i=0;i<BLOCK_SIZE-1;++i) {
		trin[index.x][index.y][i] = process_tetrahedron(ivec3(index,i),ivec3(0,0,1),ivec3(0,1,1)) + 
									process_tetrahedron(ivec3(index,i),ivec3(0,1,1),ivec3(0,1,0)) + 
									process_tetrahedron(ivec3(index,i),ivec3(0,1,0),ivec3(1,1,0)) + 
									process_tetrahedron(ivec3(index,i),ivec3(1,1,0),ivec3(1,0,0)) + 
									process_tetrahedron(ivec3(index,i),ivec3(1,0,0),ivec3(1,0,1)) + 
									process_tetrahedron(ivec3(index,i),ivec3(0,0,1),ivec3(1,0,1));
	}
}