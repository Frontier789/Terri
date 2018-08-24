#include "Block.hpp"

#include <FRONTIER/OpenGL.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

Block::Block(Block &&mv) : 
	density_shader(mv.density_shader),
	trin_shader(mv.trin_shader),
	vert_shader(mv.vert_shader),
	norm_shader(mv.norm_shader),
	shader(mv.shader),
	noiseTex1(mv.noiseTex1),
	density_buf(std::move(mv.density_buf)),
	trioff_buf(std::move(mv.trioff_buf)),
	trin_buf(std::move(mv.trin_buf)),
	tri_pos_buf(std::move(mv.tri_pos_buf)),
	tri_nrm_buf(std::move(mv.tri_nrm_buf)),
	tris_count(mv.tris_count),
	blocksize(mv.blocksize),
	rotm(mv.rotm),
	log_level(mv.log_level),
	offset(mv.offset)
{

}

Block::Block(int log_lvl,
	         ComputeShader &density_shader,
	         ComputeShader &trin_shader,
	         ComputeShader &vert_shader,
	         ComputeShader &norm_shader,
	         ShaderManager &shader,
	         Texture3D &noiseTex1) : 
	density_shader(density_shader),
	trin_shader(trin_shader),
	vert_shader(vert_shader),
	norm_shader(norm_shader),
	shader(shader),
	noiseTex1(noiseTex1),
	blocksize(33),
	log_level(log_lvl)
{
	init();
}

void Block::set_offset(vec3 p)
{
	offset = p;
	tess();
}

void Block::init() {
	init_buffers();
	checkr();
}

void Block::mulblocksize(float am) {
	blocksize = max<int>(9,min<int>(blocksize*am,1025));
	if (log_level > 1) cout << "blocksize set to " << blocksize << endl;
	init_buffers();
	checkr();
	tess();
	if (log_level > 1) cout << endl;
}

void Block::init_shader_params() {
	density_shader.setUniform("u_blocksize",blocksize);
	trin_shader.setUniform("u_blocksize",blocksize);
	vert_shader.setUniform("u_blocksize",blocksize);
}

void Block::init_buffers() {
	init_shader_params();

	r += density_buf.setData((float*)nullptr,blocksize*blocksize*blocksize);
	r += trioff_buf.setData((int*)nullptr,(blocksize-1)*(blocksize-1)*(blocksize-1));
	r += trin_buf.setData((int*)nullptr,(blocksize-1)*(blocksize-1)*(blocksize-1));
	r += density_shader.setUniform("u_noise1",noiseTex1);

	if (log_level > 1) if (r) cout << "block memory allocated" << endl;
}

void Block::checkr() {
	if (!r) {
		if (log_level > 0) cout << r << endl;
		exit(1);
	}
}

void Block::calc_density() {
	density_shader.setUniform("u_offset",offset);
	r += density_shader.setStorageBuf(3,density_buf);
	r += density_shader.dispatch(vec2(blocksize));

	checkr();
}

void Block::calc_trin() {
	r += trin_shader.setStorageBuf(3,density_buf);
	r += trin_shader.setStorageBuf(4,trin_buf);
	r += trin_shader.dispatch(vec2(blocksize-1));

	checkr();
}

template<class T>
void dump_buf_3d(Buffer &buf,int w,int h,int d,string name) {
	cout << name << ":" << endl;
	T *data = buf.map<T>();
	for (int i=0;i<w;++i) {
		for (int j=0;j<h;++j) {
			for (int k=0;k<d;++k) {
				cout << data[i*h*d + j*d + k] << " ";
			}
			cout << endl;
		}
		cout << endl;
	}

	buf.unMap();
	
	cout << endl;
}

void Block::dump_density() {
	dump_buf_3d<float>(density_buf,blocksize,blocksize,blocksize,"density data");
}

void Block::dump_trin() {
	dump_buf_3d<int>(trin_buf,blocksize-1,blocksize-1,blocksize-1,"tris count");
}

void Block::dump_trioff() {
	dump_buf_3d<int>(trioff_buf,blocksize-1,blocksize-1,blocksize-1,"tris offset");
}

void Block::dump_trinrm() {
	vec4 *data = tri_nrm_buf.map<vec4>();
	cout << "vert norms:" << endl;
	for (int i=0;i<tris_count*3;++i) {
		cout << i << ".: " << data[i] << endl;
	}

	tri_nrm_buf.unMap();
	
	cout << endl;
}

void Block::dump_verts() {
	vec4 *data = tri_pos_buf.map<vec4>();
	cout << "vert poses:" << endl;
	for (int i=0;i<tris_count*3;++i) {
		cout << i << ".: " << data[i] << endl;
	}

	tri_pos_buf.unMap();
	
	cout << endl;
}

void Block::dump_bufs() {
	dump_density();
	dump_trin();
	dump_trioff();
	dump_trinrm();
	dump_verts();

	cout << "tris count: " << tris_count << endl;
}

void Block::calc_trioff_cpu() {
	int *cnt = trin_buf.map<int>();
	int *off = trioff_buf.map<int>();
	off[0] = 0;

	for (int i=1;i<(blocksize-1)*(blocksize-1)*(blocksize-1);++i)
		off[i] = off[i-1] + cnt[i-1];
	
	tris_count = off[(blocksize-1)*(blocksize-1)*(blocksize-1)-1] + cnt[(blocksize-1)*(blocksize-1)*(blocksize-1)-1];

	trin_buf.unMap();
	trioff_buf.unMap();
	checkr();
}

void Block::create_tri_poses() {
	tri_pos_buf.setData<vec4>(nullptr,tris_count*3);
	r += vert_shader.setStorageBuf(3,density_buf);
	r += vert_shader.setStorageBuf(4,trin_buf);
	r += vert_shader.setStorageBuf(5,trioff_buf);
	r += vert_shader.setStorageBuf(6,tri_pos_buf);
	r += vert_shader.dispatch(vec2(blocksize-1));
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void Block::create_normals() {
	tri_nrm_buf.setData<vec4>(nullptr,tris_count*3);
	
	if (tris_count == 0) return;

	int onec = pow(tris_count,.333);
	
	r += norm_shader.setStorageBuf(6,tri_pos_buf);
	r += norm_shader.setStorageBuf(7,tri_nrm_buf);
	r += norm_shader.dispatch(vec3(tris_count/onec/onec,(tris_count/onec)%onec,tris_count%onec));
}

void Block::draw(ShaderManager &) {
	DrawData dd;
	dd.positions.set<vec4>(tri_pos_buf);
	dd.normals.set<vec4>(tri_nrm_buf);

	shader.getModelStack().top(rotm);

	shader.setUniform("u_offset",offset);
	shader.draw(dd);
}

void Block::rotate(vec2 d) {
	vec3 u = shader.getCamera().u();
	vec3 r = shader.getCamera().r();
	rotm = Quat(u,-d.x/80) * Quat(r,-d.y/80) * rotm;
}

void Block::zoom(float am) {
	rotm = MATRIX::scaling(vec3(am)) * rotm;
}

Time do_log(bool log,TimeQuery &tq,string task)
{
	if (!log) return Time::Zero;

	tq.stop();
	Time t = tq.getTime();
	cout << task << " calculated, took " << t.ms() << "ms" << endl;
	tq.start();

	return t;
}

void Block::tess() {
	TimeQuery tq;
	Time alltime;
	
	bool use_query = log_level > 1;

	if (use_query)
		tq.start();

	calc_density();
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); // yay
	alltime += do_log(use_query, tq, "density values");

	calc_trin();
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); // yay
	alltime += do_log(use_query, tq, "triangle numbers");

	calc_trioff_cpu();
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); // yay
	alltime += do_log(use_query, tq, "triangle offsets");
	
	create_tri_poses();
	alltime += do_log(use_query, tq, "triangle positions");
	
	create_normals();
	alltime += do_log(use_query, tq, "triangle normals");
	
	tq.stop();
	
	if (use_query) {
		cout << "tessellation took " << alltime.ms() << "ms" << endl;
		cout << "tris count " << tris_count << endl;
	}
}