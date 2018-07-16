#include <FRONTIER/OpenGL.hpp>
#include <Frontier.hpp>
#include <iostream>

using namespace std;

#define BLOCK_SIZE 5

int main()
{
	GLContext cont;
	cont.create();
	cont.setActive();

	ComputeShader density_shader;
	ComputeShader trin_shader;
	Result r;
	r += density_shader.loadFromFile("density_eval.glsl");
	r += trin_shader.loadFromFile("tris_count.glsl");


	Buffer density_buf;
	r += density_buf.setData((float*)nullptr,BLOCK_SIZE*BLOCK_SIZE*BLOCK_SIZE);

	r += density_shader.setStorageBuf(3,density_buf);
	r += density_shader.dispatch(vec2(BLOCK_SIZE));

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); // yay

	Buffer trin_buf;
	r += trin_buf.setData((int*)nullptr,(BLOCK_SIZE-1)*(BLOCK_SIZE-1)*(BLOCK_SIZE-1));

	r += trin_shader.setStorageBuf(3,density_buf);
	r += trin_shader.setStorageBuf(4,trin_buf);
	r += trin_shader.dispatch(vec2(BLOCK_SIZE-1));




	cout << "density data:" << endl;
	float *data = density_buf.map<float>();
	for (int i=0;i<BLOCK_SIZE;++i){
	for (int j=0;j<BLOCK_SIZE;++j){
	for (int k=0;k<BLOCK_SIZE;++k)
	{
		cout << data[i*BLOCK_SIZE*BLOCK_SIZE + j*BLOCK_SIZE + k] << " ";
	}cout << endl;} cout << endl;}

	density_buf.unMap();

	cout << endl;
	cout << endl;


	cout << "tirs count:" << endl;
	int *cnt = trin_buf.map<int>();
	for (int i=0;i<BLOCK_SIZE-1;++i){
	for (int j=0;j<BLOCK_SIZE-1;++j){
	for (int k=0;k<BLOCK_SIZE-1;++k)
	{
		cout << cnt[i*(BLOCK_SIZE-1)*(BLOCK_SIZE-1) + j*(BLOCK_SIZE-1) + k] << " ";
	}cout << endl;} cout << endl;}

	for (int i=1;i<(BLOCK_SIZE-1)*(BLOCK_SIZE-1)*(BLOCK_SIZE-1);++i)
		cnt[i] += cnt[i-1];
	
	int tris_all = cnt[(BLOCK_SIZE-1)*(BLOCK_SIZE-1)*(BLOCK_SIZE-1)-1]; (void)tris_all;

	for (int i=1;i<(BLOCK_SIZE-1)*(BLOCK_SIZE-1)*(BLOCK_SIZE-1);++i)
		cnt[i] -= cnt[0];
	
	cnt[0] = 0;

	trin_buf.unMap();

	cout << endl;
	cout << endl;



	cout << "tirs starts:" << endl;
	cnt = trin_buf.map<int>();
	for (int i=0;i<BLOCK_SIZE-1;++i){
	for (int j=0;j<BLOCK_SIZE-1;++j){
	for (int k=0;k<BLOCK_SIZE-1;++k)
	{
		cout << cnt[i*(BLOCK_SIZE-1)*(BLOCK_SIZE-1) + j*(BLOCK_SIZE-1) + k] << " ";
	}cout << endl;} cout << endl;}

	trin_buf.unMap();



	fg::Buffer tris_buf;

	r += tris_buf.setData<vec3>(nullptr,tris_all);



	
	if (!r) {
		cout << r << endl;
		return 1;
	}
}
