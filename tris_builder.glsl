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

#define grid_at(P) (grid[((P).x*u_blocksize + (P).y)*u_blocksize + (P).z])

vec4 lin_interp(ivec3 A,ivec3 B,float a,float b)
{
	a = abs(a);
	b = abs(b);

	return vec4((A*b + B*a) / (a+b),1)/(u_blocksize-1)*2 - 1;
}

void buildTriangle(inout int base_tp,ivec3 p1,ivec3 p2,ivec3 p3,ivec3 p4,float f1,float f2,float f3,float f4)
{
	triposes[base_tp + 0] = lin_interp(p1,p2,f1,f2);
	triposes[base_tp + 1] = lin_interp(p1,p3,f1,f3);
	triposes[base_tp + 2] = lin_interp(p1,p4,f1,f4);

	base_tp += 3;
}

void buildQuad(inout int base_tp,ivec3 p1,ivec3 p2,ivec3 p3,ivec3 p4,float f1,float f2,float f3,float f4)
{
	triposes[base_tp + 0] = lin_interp(p1,p3,f1,f3);
	triposes[base_tp + 1] = lin_interp(p2,p3,f2,f3);
	triposes[base_tp + 2] = lin_interp(p1,p4,f1,f4);
	triposes[base_tp + 3] = triposes[base_tp + 2];
	triposes[base_tp + 4] = triposes[base_tp + 1];
	triposes[base_tp + 5] = lin_interp(p2,p4,f2,f4);

	base_tp += 6;
}

void tris_for_tetrahedron(inout int base_tp,ivec3 base,ivec3 d1,ivec3 d2,float grid_base,float grid_d1,float grid_d2,float grid_1)
{
	int type = int(grid_base < 0)*1 + 
			   int(grid_d1   < 0)*2 + 
			   int(grid_d2   < 0)*4 + 
			   int(grid_1    < 0)*8;
	
	ivec3 ai,bi,ci,di;
	float af,bf,cf,df;
	int tri = 0;

	switch (type) {
		case 1:  tri = 1; ai = ivec3(0); bi = d2;       ci = d1;       di = ivec3(1);  af = grid_base; bf = grid_d2;   cf = grid_d1;   df = grid_1;  break;
		case 14: tri = 1; ai = ivec3(0); bi = d1;       ci = d2;       di = ivec3(1);  af = grid_base; bf = grid_d1;   cf = grid_d2;   df = grid_1;  break;
		case 2:  tri = 1; ai = d1;       bi = ivec3(0); ci = d2;       di = ivec3(1);  af = grid_d1;   bf = grid_base; cf = grid_d2;   df = grid_1;  break;
		case 13: tri = 1; ai = d1;       bi = d2;       ci = ivec3(0); di = ivec3(1);  af = grid_d1;   bf = grid_d2;   cf = grid_base; df = grid_1;  break;
		case 4:  tri = 1; ai = d2;       bi = d1;       ci = ivec3(0); di = ivec3(1);  af = grid_d2;   bf = grid_d1;   cf = grid_base; df = grid_1;  break;
		case 11: tri = 1; ai = d2;       bi = ivec3(0); ci = d1;       di = ivec3(1);  af = grid_d2;   bf = grid_base; cf = grid_d1;   df = grid_1;  break;
		case 8:  tri = 1; ai = ivec3(1); bi = ivec3(0); ci = d1;       di = d2;        af = grid_1;    bf = grid_base; cf = grid_d1;   df = grid_d2; break;
		case 7:  tri = 1; ai = ivec3(1); bi = d1;       ci = ivec3(0); di = d2;        af = grid_1;    bf = grid_d1;   cf = grid_base; df = grid_d2; break;
		case 3:  tri = 2; ai = ivec3(0); bi = d1;       ci = d2;       di = ivec3(1);  af = grid_base; bf = grid_d1;   cf = grid_d2;   df = grid_1;  break;
		case 12: tri = 2; ai = d1;       bi = ivec3(0); ci = d2;       di = ivec3(1);  af = grid_d1;   bf = grid_base; cf = grid_d2;   df = grid_1;  break;
		case 5:  tri = 2; ai = d2;       bi = ivec3(0); ci = d1;       di = ivec3(1);  af = grid_d2;   bf = grid_base; cf = grid_d1;   df = grid_1;  break;
		case 10: tri = 2; ai = ivec3(0); bi = d2;       ci = d1;       di = ivec3(1);  af = grid_base; bf = grid_d2;   cf = grid_d1;   df = grid_1;  break;
		case 9:  tri = 2; ai = ivec3(0); bi = ivec3(1); ci = d1;       di = d2;        af = grid_base; bf = grid_1;    cf = grid_d1;   df = grid_d2; break;
		case 6:  tri = 2; ai = ivec3(1); bi = ivec3(0); ci = d1;       di = d2;        af = grid_1;    bf = grid_base; cf = grid_d1;   df = grid_d2; break;
	}
	if (tri == 1) 
		buildTriangle(base_tp,base+ai,base+bi,base+ci,base+di,af,bf,cf,df);
	if (tri == 2)
		buildQuad(base_tp,base+ai,base+bi,base+ci,base+di,af,bf,cf,df);
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

		float grid_000 = grid_at(ivec3(index,i) + ivec3(0,0,0));
		float grid_100 = grid_at(ivec3(index,i) + ivec3(1,0,0));
		float grid_010 = grid_at(ivec3(index,i) + ivec3(0,1,0));
		float grid_001 = grid_at(ivec3(index,i) + ivec3(0,0,1));
		float grid_011 = grid_at(ivec3(index,i) + ivec3(0,1,1));
		float grid_101 = grid_at(ivec3(index,i) + ivec3(1,0,1));
		float grid_110 = grid_at(ivec3(index,i) + ivec3(1,1,0));
		float grid_111 = grid_at(ivec3(index,i) + ivec3(1,1,1));

		tris_for_tetrahedron(base,ivec3(index,i),ivec3(0,0,1),ivec3(0,1,1),grid_000,grid_001,grid_011,grid_111);
		tris_for_tetrahedron(base,ivec3(index,i),ivec3(0,1,1),ivec3(0,1,0),grid_000,grid_011,grid_010,grid_111);
		tris_for_tetrahedron(base,ivec3(index,i),ivec3(0,1,0),ivec3(1,1,0),grid_000,grid_010,grid_110,grid_111);

		tris_for_tetrahedron(base,ivec3(index,i),ivec3(1,1,0),ivec3(1,0,0),grid_000,grid_110,grid_100,grid_111);
		tris_for_tetrahedron(base,ivec3(index,i),ivec3(1,0,0),ivec3(1,0,1),grid_000,grid_100,grid_101,grid_111);
		tris_for_tetrahedron(base,ivec3(index,i),ivec3(1,0,1),ivec3(0,0,1),grid_000,grid_101,grid_001,grid_111);
	}
}
