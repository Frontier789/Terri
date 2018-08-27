#ifndef BLOCK_HPP
#define BLOCK_HPP

#include <Frontier.hpp>

class Block : NonCopyable
{
	ComputeShader *density_shader;
	ComputeShader *trin_shader;
	ComputeShader *vert_shader;
	ComputeShader *norm_shader;
	ShaderManager *shader;
	Texture3D *noiseTex1;

	Buffer density_buf;
	Buffer trioff_buf;
	Buffer trin_buf;
	Buffer tri_pos_buf;
	Buffer tri_nrm_buf;

	int tris_count;
	int blocksize;

	mat4 rotm;

	int log_level;

	vec3 offset;

public:
	Result r;

	Block(int log_lvl,
		  ComputeShader &density_shader,
		  ComputeShader &trin_shader,
		  ComputeShader &vert_shader,
		  ComputeShader &norm_shader,
		  ShaderManager &shader,
		  Texture3D &noiseTex1);
	
	Block(Block &&mv);

	Block &operator=(Block &&mv);
	
	void init();

	void set_offset(vec3 p);

	void mulblocksize(float am);
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

	fm::Size get_tris_count() const;
};

#endif