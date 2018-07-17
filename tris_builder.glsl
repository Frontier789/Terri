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

layout(std430, binding = 5) buffer trioffLayout
{
	int trioff[];
};

layout(std430, binding = 6) buffer triposLayout
{
	vec4 triposes[];
};

layout(std430, binding = 7) buffer triclrLayout
{
	vec4 triclrs[];
};

#define grid_at(P) (grid[((P).x*BLOCK_SIZE + (P).y)*BLOCK_SIZE + (P).z])

vec4 lin_interp(ivec3 A,ivec3 B)
{
	float a = abs(grid_at(A));
	float b = abs(grid_at(B));

	return vec4(((A*b + B*a) / (a+b) - vec3(BLOCK_SIZE-1)/2)/BLOCK_SIZE*2,1);
}

void buildTriangle(inout int base_tp,ivec3 p1,ivec3 p2,ivec3 p3,ivec3 p4)
{
	triposes[base_tp + 0] = lin_interp(p1,p2);
	triposes[base_tp + 1] = lin_interp(p1,p3);
	triposes[base_tp + 2] = lin_interp(p1,p4);

	vec3 c = vec3(triposes[base_tp + 0] + triposes[base_tp + 1] + triposes[base_tp + 2]) + vec3(.1);
	c = normalize(c);

	triclrs[base_tp + 0] = vec4(c,1);
	triclrs[base_tp + 1] = vec4(c,1);
	triclrs[base_tp + 2] = vec4(c,1);

	base_tp += 3;
}

void buildQuad(inout int base_tp,ivec3 p1,ivec3 p2,ivec3 p3,ivec3 p4)
{
	triposes[base_tp + 0] = lin_interp(p1,p3);
	triposes[base_tp + 1] = lin_interp(p2,p3);
	triposes[base_tp + 2] = lin_interp(p1,p4);
	triposes[base_tp + 3] = triposes[base_tp + 1];
	triposes[base_tp + 4] = triposes[base_tp + 2];
	triposes[base_tp + 5] = lin_interp(p2,p4);

	vec3 c = vec3(triposes[base_tp + 0] + triposes[base_tp + 1] + triposes[base_tp + 2] + triposes[base_tp + 5]);
	c = normalize(c);

	triclrs[base_tp + 0] = vec4(c,1);
	triclrs[base_tp + 1] = vec4(c,1);
	triclrs[base_tp + 2] = vec4(c,1);
	triclrs[base_tp + 3] = vec4(c,1);
	triclrs[base_tp + 4] = vec4(c,1);
	triclrs[base_tp + 5] = vec4(c,1);

	base_tp += 6;
}

void tris_for_tetrahedron(inout int base_tp,ivec3 base,ivec3 d1,ivec3 d2)
{
	int type = int(grid_at(base)    < 0)*1 + 
			   int(grid_at(base+d1) < 0)*2 + 
			   int(grid_at(base+d2) < 0)*4 + 
			   int(grid_at(base+ivec3(1)) < 0)*8;
	
	switch (type) {
		case 1:
		case 14: buildTriangle(base_tp,base,base+d1,base+d2,base+ivec3(1)); break;
		case 2:
		case 13: buildTriangle(base_tp,base+d1,base,base+d2,base+ivec3(1)); break;
		case 4:
		case 11: buildTriangle(base_tp,base+d2,base,base+d1,base+ivec3(1)); break;
		case 8:
		case 7:  buildTriangle(base_tp,base+ivec3(1),base,base+d1,base+d2); break;
		case 3:
		case 12: buildQuad(base_tp,base,base+d1,base+d2,base+ivec3(1)); break;
		case 5:
		case 10: buildQuad(base_tp,base,base+d2,base+d1,base+ivec3(1)); break;
		case 9:
		case 6:  buildQuad(base_tp,base,base+ivec3(1),base+d1,base+d2); break;
	}
}

void main()
{
	ivec2 index = ivec2(gl_GlobalInvocationID.xy);
	
	for (int i=0;i<BLOCK_SIZE-1;++i) {
		int flat_index = (index.x * (BLOCK_SIZE-1) + index.y) * (BLOCK_SIZE-1) + i;

		int base = trioff[flat_index]*3;

		int n = trin[flat_index]*3;

		tris_for_tetrahedron(base,ivec3(index,i),ivec3(0,0,1),ivec3(0,1,1));
		tris_for_tetrahedron(base,ivec3(index,i),ivec3(0,1,1),ivec3(0,1,0));
		tris_for_tetrahedron(base,ivec3(index,i),ivec3(0,1,0),ivec3(1,1,0));
		tris_for_tetrahedron(base,ivec3(index,i),ivec3(1,1,0),ivec3(1,0,0));
		tris_for_tetrahedron(base,ivec3(index,i),ivec3(1,0,0),ivec3(1,0,1));
		tris_for_tetrahedron(base,ivec3(index,i),ivec3(0,0,1),ivec3(1,0,1));
	}
}