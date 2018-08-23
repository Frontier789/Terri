#version 430

uniform vec3 u_offset;
uniform int u_blocksize;
uniform sampler2D u_noise1;

layout (local_size_x = 256, local_size_y = 1) in;

layout(std430, binding = 3) buffer gridLayout
{
	float grid[];
};

float height(vec2 p) {
    return texture(u_noise1,p).x;
}

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

	p.z = p.z*n;
	p.xy = vec2(dot(vec2(cos(p.z),-sin(p.z)) , p.xy),
				dot(vec2(sin(p.z), cos(p.z)) , p.xy));


	return length(p.xy*vec2(1,.5)-vec2(.04,.0))-.013;
}

float density(vec3 p)
{
	p /= (u_blocksize - 1);

	p -= vec3(.5,0,.5);


	p *= 2;
	p += u_offset;
	return p.y + texture(u_noise1, p.xz/2).x*.25 + texture(u_noise1, p.xz/4).x*.5 + texture(u_noise1, mat2(.7,.2,-.2,.7) * p.xz/8).x;

	//return min(min(solenoid(p,5,0,3),solenoid(p,5,1.9,3)),torus(p,0,3));

	//return (p.z - .5) + height(p.xy)*.003 + height(p.xy/10)*.03 + height(p.xy/40 + vec2(.1))*.12;
}

void main()
{
	ivec2 index = ivec2(gl_WorkGroupID.xy);
	int size  = int(max((u_blocksize+1) / gl_WorkGroupSize.x,1));
	int loc   = int(gl_LocalInvocationID.x);
	int start = int(size * loc);
	int end   = int(min(start + size,u_blocksize));

	for (int i=start;i<end;++i) {
		vec3 p = vec3(index,i);

		float d = density(p);

		grid[(index.x*u_blocksize + index.y)*u_blocksize + i] = d;
	}
}