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

#define grid_at(P) (grid[((P).x*u_blocksize + (P).y)*u_blocksize + (P).z])

vec4 lin_interp(ivec3 A,ivec3 B)
{
	float a = abs(grid_at(A));
	float b = abs(grid_at(B));

	return vec4(((A*b + B*a) / (a+b) - vec3(u_blocksize-1)/2)/u_blocksize*2,1);
}

vec3 normal(int base,int d1,int d2,int d3)
{
	vec3 A = vec3(triposes[base + d1]) - vec3(triposes[base + d2]);
	vec3 B = vec3(triposes[base + d1]) - vec3(triposes[base + d3]);

	vec3 n = normalize(cross(A,B));

	return n;
}

void buildTriangle(inout int base_tp,ivec3 p1,ivec3 p2,ivec3 p3,ivec3 p4)
{
	triposes[base_tp + 0] = lin_interp(p1,p2);
	triposes[base_tp + 1] = lin_interp(p1,p3);
	triposes[base_tp + 2] = lin_interp(p1,p4);

	vec3 c = normal(base_tp,0,1,2);

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
	triposes[base_tp + 3] = triposes[base_tp + 2];
	triposes[base_tp + 4] = triposes[base_tp + 1];
	triposes[base_tp + 5] = lin_interp(p2,p4);

	vec3 c = normal(base_tp,0,1,2);

	triclrs[base_tp + 0] = vec4(c,1);
	triclrs[base_tp + 1] = vec4(c,1);
	triclrs[base_tp + 2] = vec4(c,1);

	c = normal(base_tp,3,4,5);

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
	
	ivec3 ai,bi,ci,di;
	int tri = 0;

	switch (type) {
		case 1:  tri = 1; ai = ivec3(0); bi = d2;       ci = d1;       di = ivec3(1); break;
		case 14: tri = 1; ai = ivec3(0); bi = d1;       ci = d2;       di = ivec3(1); break;
		case 2:  tri = 1; ai = d1;       bi = ivec3(0); ci = d2;       di = ivec3(1); break;
		case 13: tri = 1; ai = d1;       bi = d2;       ci = ivec3(0); di = ivec3(1); break;
		case 4:  tri = 1; ai = d2;       bi = d1;       ci = ivec3(0); di = ivec3(1); break;
		case 11: tri = 1; ai = d2;       bi = ivec3(0); ci = d1;       di = ivec3(1); break;
		case 8:  tri = 1; ai = ivec3(1); bi = ivec3(0); ci = d1;       di = d2;       break;
		case 7:  tri = 1; ai = ivec3(1); bi = d1;       ci = ivec3(0); di = d2;       break;
		case 3:  tri = 2; ai = ivec3(0); bi = d1;       ci = d2;       di = ivec3(1); break;
		case 12: tri = 2; ai = d1;       bi = ivec3(0); ci = d2;       di = ivec3(1); break;
		case 5:  tri = 2; ai = d2;       bi = ivec3(0); ci = d1;       di = ivec3(1); break;
		case 10: tri = 2; ai = ivec3(0); bi = d2;       ci = d1;       di = ivec3(1); break;
		case 9:  tri = 2; ai = ivec3(0); bi = ivec3(1); ci = d1;       di = d2;       break;
		case 6:  tri = 2; ai = ivec3(1); bi = ivec3(0); ci = d1;       di = d2;       break;
	}
	if (tri == 1)
		buildTriangle(base_tp,base+ai,base+bi,base+ci,base+di);
	if (tri == 2)
		buildQuad(base_tp,base+ai,base+bi,base+ci,base+di);
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

		int base = trioff[flat_index]*3;

		int n = trin[flat_index]*3;

		tris_for_tetrahedron(base,ivec3(index,i),ivec3(0,0,1),ivec3(0,1,1));
		tris_for_tetrahedron(base,ivec3(index,i),ivec3(0,1,1),ivec3(0,1,0));
		tris_for_tetrahedron(base,ivec3(index,i),ivec3(0,1,0),ivec3(1,1,0));

		tris_for_tetrahedron(base,ivec3(index,i),ivec3(1,1,0),ivec3(1,0,0));
		tris_for_tetrahedron(base,ivec3(index,i),ivec3(1,0,0),ivec3(1,0,1));
		tris_for_tetrahedron(base,ivec3(index,i),ivec3(1,0,1),ivec3(0,0,1));
	}
}