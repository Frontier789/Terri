#include <FRONTIER/OpenGL.hpp>
#include <Frontier.hpp>
#include <iostream>

using namespace std;

#define BLOCK_SIZE 17

class App
{
	ComputeShader density_shader;
	ComputeShader trin_shader;
	ComputeShader vert_shader;

	Buffer density_buf;
	Buffer trioff_buf;
	Buffer trin_buf;
	Buffer tri_pos_buf;
	Buffer tri_clr_buf;

	int tris_count;

	mat4 rotm;

public:
	Result r;

	App() {
		init();
	}

	void init() {
		init_shaders();
		init_buffers();
		checkr();
	}

	void init_shaders() {
		r += density_shader.loadFromFile("density_eval.glsl");
		r += trin_shader.loadFromFile("tris_count.glsl");
		r += vert_shader.loadFromFile("tris_builder.glsl");
	}

	void init_buffers() {
		r += density_buf.setData((float*)nullptr,BLOCK_SIZE*BLOCK_SIZE*BLOCK_SIZE);
		r += trioff_buf.setData((int*)nullptr,(BLOCK_SIZE-1)*(BLOCK_SIZE-1)*(BLOCK_SIZE-1));
		r += trin_buf.setData((int*)nullptr,(BLOCK_SIZE-1)*(BLOCK_SIZE-1)*(BLOCK_SIZE-1));
	}

	void checkr() {
		if (!r) {
			cout << r << endl;
			exit(1);
		}
	}

	void calc_density() {
		r += density_shader.setStorageBuf(3,density_buf);
		r += density_shader.dispatch(vec2(BLOCK_SIZE));
	}

	void calc_trin() {
		r += trin_shader.setStorageBuf(3,density_buf);
		r += trin_shader.setStorageBuf(4,trin_buf);
		r += trin_shader.dispatch(vec2(BLOCK_SIZE-1));
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

	void dump_density() {
		dump_buf_3d<float>(density_buf,BLOCK_SIZE,BLOCK_SIZE,BLOCK_SIZE,"density data");
	}

	void dump_trin() {
		dump_buf_3d<int>(trin_buf,BLOCK_SIZE-1,BLOCK_SIZE-1,BLOCK_SIZE-1,"tris count");
	}

	void dump_trioff() {
		dump_buf_3d<int>(trioff_buf,BLOCK_SIZE-1,BLOCK_SIZE-1,BLOCK_SIZE-1,"tris offset");
	}

	void dump_verts() {
		vec4 *data = tri_pos_buf.map<vec4>();
		cout << "vert poses:" << endl;
		for (int i=0;i<tris_count*3;++i) {
			cout << i << ".: " << data[i] << endl;
		}

		tri_pos_buf.unMap();
		
		cout << endl;
	}

	void dump_bufs() {
		dump_density();
		dump_trin();
		dump_trioff();

		cout << "tris count: " << tris_count << endl;
	}

	void calc_trioff_cpu() {
		int *cnt = trin_buf.map<int>();
		int *off = trioff_buf.map<int>();
		off[0] = 0;

		for (int i=1;i<(BLOCK_SIZE-1)*(BLOCK_SIZE-1)*(BLOCK_SIZE-1);++i)
			off[i] = off[i-1] + cnt[i-1];
		
		tris_count = off[(BLOCK_SIZE-1)*(BLOCK_SIZE-1)*(BLOCK_SIZE-1)-1] + cnt[(BLOCK_SIZE-1)*(BLOCK_SIZE-1)*(BLOCK_SIZE-1)-1];

		trin_buf.unMap();
		trioff_buf.unMap();
		checkr();
	}

	void create_tri_poses() {
		tri_pos_buf.setData<vec4>(nullptr,tris_count*3);
		tri_clr_buf.setData<vec4>(nullptr,tris_count*3);
		r += vert_shader.setStorageBuf(3,density_buf);
		r += vert_shader.setStorageBuf(4,trin_buf);
		r += vert_shader.setStorageBuf(5,trioff_buf);
		r += vert_shader.setStorageBuf(6,tri_pos_buf);
		r += vert_shader.setStorageBuf(7,tri_clr_buf);
		r += vert_shader.dispatch(vec2(BLOCK_SIZE-1));
	}

	void draw(ShaderManager &shader) {
		DrawData dd;
		dd.positions.set<vec4>(tri_pos_buf);
		dd.colors.set<vec4>(tri_clr_buf);

		shader.getCamera().set3D(vec2(1280,1024),vec3(0,0,5),vec3());
		shader.getModelStack().top(rotm);

		shader.draw(dd);
	}

	void rotate(vec3 u,vec3 r,vec2 d) {
		rotm = Quat(u,-d.x/80) * Quat(r,-d.y/80) * rotm;
	}

	void zoom(float am) {
		rotm = MATRIX::scaling(vec3(am)) * rotm;
	}
};

class Widget : public GuiElement, public ClickListener, public ScrollListener
{
public:
	fm::Delegate<void,fg::ShaderManager &> ondraw; ///< Callback used when drawing
	fm::Delegate<void,fm::vec2,fm::vec2> onmousemove; ///< Callback used when mouse moves
	fm::Delegate<void,float> onscroll; ///< Callback used when scrolling happens

	/////////////////////////////////////////////////////////////
	/// @brief Default constructor
	/// 
	/// @param owner The owner context
	/// @param size The initial size
	///
	/////////////////////////////////////////////////////////////
	Widget(GuiContext &owner,fm::vec2 size = fm::vec2());

	/////////////////////////////////////////////////////////////
	/// @brief draw the gui element
	///
	/// @param shader The shader to use
	///
	/////////////////////////////////////////////////////////////
	virtual void onDraw(fg::ShaderManager &shader) override;
	
	/////////////////////////////////////////////////////////////
	/// @brief Called when the mouse moves inside the gui element
	/// 
	/// @param p The position of the mouse after moving
	/// @param prevP The position of the mouse before moving
	/// 
	/////////////////////////////////////////////////////////////
	virtual void onMouseMove(fm::vec2 p,fm::vec2 prevP) override;
		
	/////////////////////////////////////////////////////////////
	/// @brief Called when the element is scrolled
	/// 
	/// @param amount The amount the element is scrolled
	/// 
	/////////////////////////////////////////////////////////////
	virtual void onScroll(float amount) override;
};

Widget::Widget(GuiContext &owner,fm::vec2 size) : GuiElement(owner,size) {}

/////////////////////////////////////////////////////////////
void Widget::onDraw(fg::ShaderManager &shader)
{
	GuiElement::onDraw(shader);
	ondraw(shader);
}

/////////////////////////////////////////////////////////////
void Widget::onMouseMove(fm::vec2 p,fm::vec2 prevP)
{
	ClickListener::onMouseMove(p,prevP);
	onmousemove(p,prevP);
}

/////////////////////////////////////////////////////////////
void Widget::onScroll(float amount)
{
	ScrollListener::onScroll(amount);
	onscroll(amount);
}

int main()
{
	GuiWindow win(vec2(1280,1024));
	win.setClearColor(vec4::Black);
	win.setDepthTest(LEqual);

	App app;
	cout << "init: ok" << endl;

	app.calc_density();
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); // yay

	app.calc_trin();
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); // yay

	app.calc_trioff_cpu();
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); // yay
	
	app.create_tri_poses();

	// app.dump_bufs();

	Camera &cam = win.getShader().getCamera();

	Widget *w = new Widget(win,win.getSize());
	w->ondraw = [&](ShaderManager &shader) {
		app.draw(shader);
	};
	w->onmousemove = [&](vec2 a,vec2 b) {
		if (w->isPressed(Mouse::Left))
			app.rotate(cam.u(),cam.r(),a-b);
	};
	w->onscroll = [&](float d) {
		app.zoom(pow(1.3,d));
	};
	win.getMainLayout().addChildElement(w);

	win.runGuiLoop();
}
