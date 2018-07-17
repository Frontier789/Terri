#version 430

#define BLOCK_SIZE 17

layout (local_size_x = 1, local_size_y = 1) in;

layout(std430, binding = 3) buffer gridLayout
{
	float grid[];
};

layout(std430, binding = 4) buffer trinLayout
{
	int trin[];
};

#define grid_at(P) (grid[((P).x*BLOCK_SIZE + (P).y)*BLOCK_SIZE + (P).z])

int process_tetrahedron(ivec3 base,ivec3 d1,ivec3 d2)
{
	int n = int(grid_at(base) < 0) + 
			int(grid_at(base+d1) < 0) + 
			int(grid_at(base+d2) < 0) + 
			int(grid_at(base+ivec3(1)) < 0);

	return 2-abs(n-2);
}

void main()
{
	ivec2 index = ivec2(gl_GlobalInvocationID.xy);
	
	for (int i=0;i<BLOCK_SIZE-1;++i) {
		int flat_index = (index.x * (BLOCK_SIZE-1) + index.y) * (BLOCK_SIZE-1) + i;

		trin[flat_index] = 
			process_tetrahedron(ivec3(index,i),ivec3(0,0,1),ivec3(0,1,1)) + 
			process_tetrahedron(ivec3(index,i),ivec3(0,1,1),ivec3(0,1,0)) + 
			process_tetrahedron(ivec3(index,i),ivec3(0,1,0),ivec3(1,1,0)) + 
			process_tetrahedron(ivec3(index,i),ivec3(1,1,0),ivec3(1,0,0)) + 
			process_tetrahedron(ivec3(index,i),ivec3(1,0,0),ivec3(1,0,1)) + 
			process_tetrahedron(ivec3(index,i),ivec3(0,0,1),ivec3(1,0,1));
	}
}