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
	ivec2 index = ivec2(gl_WorkGroupID.xy);
	int size  = int(max(u_blocksize / gl_WorkGroupSize.x,1));
	int loc   = int(gl_LocalInvocationID.x);
	int start = int(size * loc);
	int end   = int(min(start + size,u_blocksize-1));

	for (int i=start;i<end;++i) {
		int flat_index = (index.x * (u_blocksize-1) + index.y) * (u_blocksize-1) + i;

		trin[flat_index] = 
			process_tetrahedron(ivec3(index,i),ivec3(0,0,1),ivec3(0,1,1)) + 
			process_tetrahedron(ivec3(index,i),ivec3(0,1,1),ivec3(0,1,0)) + 
			process_tetrahedron(ivec3(index,i),ivec3(0,1,0),ivec3(1,1,0)) + 
			process_tetrahedron(ivec3(index,i),ivec3(1,1,0),ivec3(1,0,0)) + 
			process_tetrahedron(ivec3(index,i),ivec3(1,0,0),ivec3(1,0,1)) + 
			process_tetrahedron(ivec3(index,i),ivec3(0,0,1),ivec3(1,0,1));
	}
}