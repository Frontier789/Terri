#version 430

uniform vec3 u_offset;
uniform int u_blocksize;
uniform sampler3D u_noise1;

layout (local_size_x = 256, local_size_y = 1) in;

layout(std430, binding = 3) buffer gridLayout
{
	float grid[];
};

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

vec4 noise(vec3 p)
{
	return texture(u_noise1, p)/255.0;
}

float density(vec3 p)
{
	p /= (u_blocksize - 1);

	p -= vec3(.5,0,.5);

	// return length(p*2 + u_offset) - 1.5 + texture(u_noise1, p*2).x/25500.0;

	p *= 2;
	p += u_offset;
	float s = sin(1.45);
	float c = cos(1.45);
	vec3 prot = mat3(c,0,-s,0,1,0,s,0,c)*p;
	return p.y + noise(prot/8).x*400
	           + noise(p/2).y*50
			   + noise(p*1).z*20
			   + noise(p*2).w*10;

	// return min(min(solenoid(p,5,0,3),solenoid(p,5,1.9,3)),torus(p,0,3));
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