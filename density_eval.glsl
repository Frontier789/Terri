#version 430

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

float density(vec3 p)
{
	p /= (u_blocksize - 1);
/*
	float M=15;

	return sin(M*p.x + .5) + sin(M*p.y + 1) + sin(M*p.z + 3);*/

	return (p.z - .5) + height(p.xy)*.003 + height(p.xy/10)*.03 + height(p.xy/40 + vec2(.1))*.12;
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