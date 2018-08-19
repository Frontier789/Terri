#ifndef BLOCK_HPP
#define BLOCK_HPP

#include <Frontier.hpp>

class Block
{
	ComputeShader density_shader;
	ComputeShader trin_shader;
	ComputeShader vert_shader;
	ComputeShader norm_shader;

	Buffer density_buf;
	Buffer trioff_buf;
	Buffer trin_buf;
	Buffer tri_pos_buf;
	Buffer tri_nrm_buf;

	int tris_count;
	int blocksize;

	mat4 rotm;

	Texture noiseTex1;
	ShaderManager shader;
	
	int log_level;

public:
	Result r;

	Block(int log_lvl = 1);
	void init();

	void mulblocksize(float am);
	void init_textures();
	void init_shaders();
	void init_draw_shader();
	void init_shader_params();
	void init_buffers();
	
	void checkr();
	
	void calc_density();
	void calc_trin();
	
	void dump_density();
	void dump_trin();
	void dump_trioff();
	void dump_trinrm();
	void dump_verts();
	void dump_bufs();
	
	void calc_trioff_cpu();
	void create_tri_poses();
	void create_normals();
	
	void draw(ShaderManager &);
	
	void rotate(vec2 d);
	void zoom(float am);
	void tess();
	void set_time(Time t);
};

#endif