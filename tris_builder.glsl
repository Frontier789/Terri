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

float torus(vec3 p,float aoff,float s)
{
	p.xz = vec2(length(p.xz)/s,atan(p.z,p.x) + aoff);
	p.xy -= vec2(.1,.5);
	p.y /= s;

	return length(p.xy)-.013;
}

float solenoid(vec3 p,float n,float aoff,float s)
{
	p.xz = vec2(length(p.xz)/s,atan(p.z,p.x) + aoff);
	p.xy -= vec2(.1,.5);
	p.y /= s;

	p.z *= n;
	p.xy = vec2(dot(vec2(cos(p.z),-sin(p.z)) , p.xy),
				dot(vec2(sin(p.z), cos(p.z)) , p.xy));


	return length(p.xy*vec2(1,.5)-vec2(.04,.0))-.013;
}

float density(vec3 p)
{
	p /= (u_blocksize - 1);

	p -= vec3(.5,0,.5);

	return min(min(solenoid(p,5,0,3),solenoid(p,5,1.9,3)),torus(p,0,3));

	//return (p.z - .5) + height(p.xy)*.003 + height(p.xy/10)*.03 + height(p.xy/40 + vec2(.1))*.12;
}


vec4 normal_grid(vec4 P)
{
	float e = 1;
	return
		vec4(density(P.xyz+vec3(e,0,0))-density(P.xyz-vec3(e,0,0)),
			 density(P.xyz+vec3(0,e,0))-density(P.xyz-vec3(0,e,0)),
 			 density(P.xyz+vec3(0,0,e))-density(P.xyz-vec3(0,0,e)),1);
}

vec4 lin_interp(ivec3 A,ivec3 B)
{
	float a = abs(grid_at(A));
	float b = abs(grid_at(B));

	return vec4((A*b + B*a) / (a+b),1);
}

void buildTriangle(inout int base_tp,ivec3 p1,ivec3 p2,ivec3 p3,ivec3 p4)
{
	triposes[base_tp + 0] = lin_interp(p1,p2)/u_blocksize*2 - 1;
	triposes[base_tp + 1] = lin_interp(p1,p3)/u_blocksize*2 - 1;
	triposes[base_tp + 2] = lin_interp(p1,p4)/u_blocksize*2 - 1;

	triclrs[base_tp + 0] = normal_grid(lin_interp(p1,p2));
	triclrs[base_tp + 1] = normal_grid(lin_interp(p1,p3));
	triclrs[base_tp + 2] = normal_grid(lin_interp(p1,p4));

	base_tp += 3;
}

void buildQuad(inout int base_tp,ivec3 p1,ivec3 p2,ivec3 p3,ivec3 p4)
{
	triposes[base_tp + 0] = lin_interp(p1,p3)/u_blocksize*2 - 1;
	triposes[base_tp + 1] = lin_interp(p2,p3)/u_blocksize*2 - 1;
	triposes[base_tp + 2] = lin_interp(p1,p4)/u_blocksize*2 - 1;
	triposes[base_tp + 3] = triposes[base_tp + 2];
	triposes[base_tp + 4] = triposes[base_tp + 1];
	triposes[base_tp + 5] = lin_interp(p2,p4)/u_blocksize*2 - 1;

	triclrs[base_tp + 0] = normal_grid(lin_interp(p1,p3));
	triclrs[base_tp + 1] = normal_grid(lin_interp(p2,p3));
	triclrs[base_tp + 2] = normal_grid(lin_interp(p1,p4));
	triclrs[base_tp + 3] = normal_grid(lin_interp(p1,p4));
	triclrs[base_tp + 4] = normal_grid(lin_interp(p2,p3));
	triclrs[base_tp + 5] = normal_grid(lin_interp(p2,p4));

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