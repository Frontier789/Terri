#version 430

in vec3 va_nrm;
in vec3 va_clr;

out vec4 out_clr;

vec3 sun_dir = normalize(vec3(-3,9,4));

void main()
{
	vec3 n = normalize(va_nrm);
	
	float dp = max(dot(sun_dir,n),0.1);
	
	out_clr = vec4(va_clr*dp,1);
}