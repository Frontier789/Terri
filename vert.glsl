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

in vec3 FRONTIER_POS    in_pos;
in vec3 FRONTIER_NRM    in_nrm;

out vec3 va_nrm;
out vec3 va_clr;

vec3 height_clr(float h);

void main()
{
	va_nrm = vec3(u_normMat * vec4(in_nrm,1));

	va_clr = height_clr(in_pos.z);
	
	gl_Position = u_projMat * u_viewMat * u_modelMat * vec4(in_pos,1.0);
}

vec3 height_clr(float h)
{
	h = (h + .3) / .2;
	if (h < .3) return mix(vec3(.2,.2,.5),vec3(.5,.5,.8),h/.3);
	if (h < .6) return mix(vec3(.5,.5,.8),vec3(.2,.5,.3),(h-.3)/.2);
	if (h < .8) return mix(vec3(.2,.5,.3),vec3(.4,.3,.2),(h-.6)/.2);
	if (h < .95) return mix(vec3(.4,.3,.2),vec3(.4,.2,.1),(h-.6)/.2);
	return vec3(h);
}