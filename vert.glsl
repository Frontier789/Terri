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
out vec3 height_clr;

vec3 hclr(float h);

void main()
{
	va_nrm = vec3(u_normMat * vec4(in_nrm,0));

	vec4 p = u_modelMat * vec4(in_pos + u_offset,1.0);
	
	height_clr = hclr(p.y + (sin(p.x) + cos(p.z))*.2);

	gl_Position = u_projMat * u_viewMat * p;
}

vec3 interp(float a,float b,float x,vec3 A,vec3 B)
{
	return mix(A,B,(x-a)/(b-a));
}

vec3 hclr(float h)
{
	float r = (h - (-2.5)) / (-1.5 - (-2.5));

	const vec3 white = vec3(1,1,1);
	const vec3 brown = vec3(.62,.34,.26);
	const vec3 light_brown = vec3(.69,.4,.28);
	const vec3 butter = vec3(.84,.66,.48);
	const vec3 lime = vec3(.72,.83,.46);
	const vec3 green = vec3(.55,.69,.39);
	const vec3 light_cyan = vec3(.88,.99,1);
	const vec3 cyan = vec3(.58,.79,.91);
	const vec3 dark_cyan = vec3(.25,.67,.89);
	const vec3 blue = vec3(.15,.27,.49);
	const vec3 dark_blue = vec3(.05,.17,.29);

	if (r > 1) return white;
	if (r > .9) return interp( 1,.9,r,white,brown);
	if (r > .8) return interp(.9,.8,r,brown,light_brown);
	if (r > .7) return interp(.8,.7,r,light_brown,butter);
	if (r > .6) return interp(.7,.6,r,butter,lime);
	if (r > .5) return interp(.6,.5,r,lime,green);
	if (r > .4) return interp(.5,.4,r,green,light_cyan);
	if (r > .3) return interp(.4,.3,r,light_cyan,cyan);
	if (r > .2) return interp(.3,.2,r,cyan,dark_cyan);
	if (r > .1) return interp(.2,.1,r,dark_cyan,blue);
	if (r > .0) return interp(.1,.0,r,blue,dark_blue);
	return dark_blue;
}
