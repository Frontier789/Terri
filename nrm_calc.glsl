#version 430

uniform int u_blocksize;
uniform int u_triscount;

layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 6) buffer triposLayout
{
	vec4 triposes[];
};

layout(std430, binding = 7) buffer trinrmLayout
{
	vec4 trinrms[];
};

#define global_id    gl_GlobalInvocationID
#define work_groups  gl_NumWorkGroups

void main()
{
	int base = int(global_id.x*work_groups.y*work_groups.z + global_id.y*work_groups.z + global_id.z)*3;
	vec3 A = vec3(triposes[base + 0]);
	vec3 B = vec3(triposes[base + 1]);
	vec3 C = vec3(triposes[base + 2]);
	vec4 n = vec4(normalize(cross(A-B,A-C)),1);
	trinrms[base + 0] = n;
	trinrms[base + 1] = n;
	trinrms[base + 2] = n;
}
