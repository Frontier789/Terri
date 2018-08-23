#version 430

#define FRONTIER_MODEL
#define FRONTIER_VIEW
#define FRONTIER_PROJ
#define FRONTIER_POS
#define FRONTIER_NRM
#define FRONTIER_NRMMAT
#define FRONTIER_SECONDS

uniform mat4 FRONTIER_NRMMAT u_normMat;
uniform mat4 FRONTIER_MODEL  u_modelMat;
uniform mat4 FRONTIER_VIEW   u_viewMat;
uniform mat4 FRONTIER_PROJ   u_projMat;
uniform float FRONTIER_SECONDS u_secs;

uniform vec3 u_offset;

in vec3 FRONTIER_POS    in_pos;
in vec3 FRONTIER_NRM    in_nrm;

out vec3 va_nrm;

vec3 height_clr(float h);

void main()
{
	va_nrm = vec3(u_normMat * vec4(in_nrm,0));
	
	gl_Position = u_projMat * u_viewMat * u_modelMat * vec4(in_pos + u_offset,1.0);
}
