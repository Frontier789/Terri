#version 430

uniform int u_blocksize;

layout (local_size_x = 256, local_size_y = 1) in;

layout(std430, binding = 3) buffer gridLayout
{
	float grid[];
};

layout(std430, binding = 4) buffer trinLayout
{
	int trin[];
};

#define grid_at(P) (grid[((P).x*u_blocksize + (P).y)*u_blocksize + (P).z])

int process_tetrahedron(float grid_base,float grid_d1,float grid_d2,float grid_1)
{
	int n = int(grid_base < 0) + 
			int(grid_d1   < 0) + 
			int(grid_d2   < 0) + 
			int(grid_1    < 0);

	return 2-abs(n-2);
}

void main()
{
	ivec2 index = ivec2(gl_WorkGroupID.xy);
	int size  = int(max(u_blocksize / gl_WorkGroupSize.x,1));
	int loc   = int(gl_LocalInvocationID.x);
	int start = int(size * loc);
	int end   = int(min(start + size,u_blocksize-1));

	for (int i=start;i<end;++i) {
		int flat_index = (index.x * (u_blocksize-1) + index.y) * (u_blocksize-1) + i;

		float grid_000 = grid_at(ivec3(index,i) + ivec3(0,0,0));
		float grid_100 = grid_at(ivec3(index,i) + ivec3(1,0,0));
		float grid_010 = grid_at(ivec3(index,i) + ivec3(0,1,0));
		float grid_001 = grid_at(ivec3(index,i) + ivec3(0,0,1));
		float grid_110 = grid_at(ivec3(index,i) + ivec3(1,1,0));
		float grid_101 = grid_at(ivec3(index,i) + ivec3(1,0,1));
		float grid_011 = grid_at(ivec3(index,i) + ivec3(0,1,1));
		float grid_111 = grid_at(ivec3(index,i) + ivec3(1,1,1));

		trin[flat_index] = 
			process_tetrahedron(grid_000,grid_001,grid_011,grid_111) + 
			process_tetrahedron(grid_000,grid_011,grid_010,grid_111) + 
			process_tetrahedron(grid_000,grid_010,grid_110,grid_111) + 
			process_tetrahedron(grid_000,grid_110,grid_100,grid_111) + 
			process_tetrahedron(grid_000,grid_100,grid_101,grid_111) + 
			process_tetrahedron(grid_000,grid_101,grid_001,grid_111);
	}
}