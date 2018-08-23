#ifndef BLOCK_FACTORY_HPP
#define BLOCK_FACTORY_HPP
#include "Block.hpp"

#include <Frontier.hpp>

class BlockFactory
{
	ComputeShader density_shader;
	ComputeShader trin_shader;
	ComputeShader vert_shader;
	ComputeShader norm_shader;

	ShaderManager shader;
	Texture noiseTex1;
	int log_level;
public:

	BlockFactory(int log_lvl = 1);
	Result init();

	Block createBlock();
	
	Result init_textures();
	Result init_shaders();
	Result init_draw_shader();

	Camera &getCam();
};

#endif