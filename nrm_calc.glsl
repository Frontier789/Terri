#version 430

uniform int u_blocksize;
uniform int u_triscount;

layout (local_size_x = 256, local_size_y = 1) in;

layout(std430, binding = 6) buffer triposLayout
{
	vec4 triposes[];
};

layout(std430, binding = 7) buffer trinrmLayout
{
	vec4 trinrms[];
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
}

vec4 normal(vec4 P)
{
	float e = 1;
	return
		vec4(density(P.xyz+vec3(e,0,0))-density(P.xyz-vec3(e,0,0)),
			 density(P.xyz+vec3(0,e,0))-density(P.xyz-vec3(0,e,0)),
 			 density(P.xyz+vec3(0,0,e))-density(P.xyz-vec3(0,0,e)),1);
}

void main()
{
	trinrms[gl_GlobalInvocationID.x] = normal()
}
