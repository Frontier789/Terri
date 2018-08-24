#include "BlockFactory.hpp"
#include <iostream>
using namespace std;

BlockFactory::BlockFactory(int log_lvl) : log_level(log_lvl) {}

Result BlockFactory::init()
{
	Result r;

	r += init_shaders();
	r += init_textures();
	r += init_draw_shader();

	return r;
}

Block BlockFactory::createBlock()
{
	return Block(log_level,density_shader,trin_shader,vert_shader,norm_shader,shader,noiseTex1);
}

Result BlockFactory::init_textures() {
	srand(time(0));

	Color clr[16*16*16];
	C(16*16*16) clr[i].rgb() = vec3(rand()%256,rand()%256,rand()%256);

	Result r = noiseTex1.loadFromMemory(vec3s(16),clr);

	noiseTex1.setSmooth(true);
	noiseTex1.setRepeated(true);
	return r;
}

Result BlockFactory::init_shaders() {
	Clock clk;
	Result r;
	r += density_shader.loadFromFile("density_eval.glsl");
	r += trin_shader.loadFromFile("tris_count.glsl");
	r += vert_shader.loadFromFile("tris_builder.glsl");
	r += norm_shader.loadFromFile("nrm_calc.glsl");

	if (log_level > 1) cout << "Shader compilation took " << clk.s()*1000 << "ms" << endl;

	init_draw_shader();

	if (log_level > 1) if (r) cout << "shaders loaded" << endl;

	return r;
}

Result BlockFactory::init_draw_shader() {
	Result r;
	
	auto &cam = shader.getCamera();

	vec3 p = cam.getPosition();
	vec3 v = cam.getViewDir();

	r += shader.loadFromFiles("vert.glsl","frag.glsl");
	
	if (!r && log_level > 0) 
		cout << r << endl;
	
	cam.set3D(vec2(640,480),p,v + p);
	
	return r;
}

Camera &BlockFactory::getCam()
{
	return shader.getCamera();
}
