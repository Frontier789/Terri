#include <FRONTIER/OpenGL.hpp>
#include <Frontier.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

class App
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

public:
	Result r;

	App() : blocksize(33) {
		init();
	}

	void init() {
		init_textures();
		init_shaders();
		init_buffers();
		checkr();
	}

	void mulblocksize(float am) {
		blocksize = max<int>(9,min<int>(blocksize*am,1025));
		cout << blocksize << endl;
		init_buffers();
		checkr();
		tess();
	}

	void init_textures() {
		r += noiseTex1.loadFromFile("noise.jpg");
		noiseTex1.setSmooth(true);
	}

	void init_shaders() {
		Clock clk;
		r += density_shader.loadFromFile("density_eval.glsl");
		r += trin_shader.loadFromFile("tris_count.glsl");
		r += vert_shader.loadFromFile("tris_builder.glsl");
		r += norm_shader.loadFromFile("nrm_calc.glsl");

		cout << "Shader compilation took " << clk.s()*1000 << "ms" << endl;

		init_shader_params();

		init_draw_shader();

		if (r) cout << "shaders loaded" << endl;
	}

	void init_draw_shader() {
		r += shader.loadFromFiles("vert.glsl","frag.glsl");
		if (!r) {
			cout << r << endl;
			r = Result();
		}
	}

	void init_shader_params() {
		density_shader.setUniform("u_blocksize",blocksize);
		trin_shader.setUniform("u_blocksize",blocksize);
		vert_shader.setUniform("u_blocksize",blocksize);
	}

	void init_buffers() {
		init_shader_params();

		r += density_buf.setData((float*)nullptr,blocksize*blocksize*blocksize);
		r += trioff_buf.setData((int*)nullptr,(blocksize-1)*(blocksize-1)*(blocksize-1));
		r += trin_buf.setData((int*)nullptr,(blocksize-1)*(blocksize-1)*(blocksize-1));
		density_shader.setUniform("u_noise1",noiseTex1);

		if (r) cout << "memory allocated" << endl;
	}

	void checkr() {
		if (!r) {
			cout << r << endl;
			exit(1);
		}
	}

	void calc_density() {
		r += density_shader.setStorageBuf(3,density_buf);
		r += density_shader.dispatch(vec2(blocksize));
	}

	void calc_trin() {
		r += trin_shader.setStorageBuf(3,density_buf);
		r += trin_shader.setStorageBuf(4,trin_buf);
		r += trin_shader.dispatch(vec2(blocksize-1));
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
		dump_buf_3d<float>(density_buf,blocksize,blocksize,blocksize,"density data");
	}

	void dump_trin() {
		dump_buf_3d<int>(trin_buf,blocksize-1,blocksize-1,blocksize-1,"tris count");
	}

	void dump_trioff() {
		dump_buf_3d<int>(trioff_buf,blocksize-1,blocksize-1,blocksize-1,"tris offset");
	}

	void dump_trinrm() {
		vec4 *data = tri_nrm_buf.map<vec4>();
		cout << "vert norms:" << endl;
		for (int i=0;i<tris_count*3;++i) {
			cout << i << ".: " << data[i] << endl;
		}

		tri_nrm_buf.unMap();
		
		cout << endl;
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
		dump_trinrm();
		dump_verts();

		cout << "tris count: " << tris_count << endl;
	}

	void calc_trioff_cpu() {
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

	void create_tri_poses() {
		tri_pos_buf.setData<vec4>(nullptr,tris_count*3);
		tri_nrm_buf.setData<vec4>(nullptr,tris_count*3);
		r += vert_shader.setStorageBuf(3,density_buf);
		r += vert_shader.setStorageBuf(4,trin_buf);
		r += vert_shader.setStorageBuf(5,trioff_buf);
		r += vert_shader.setStorageBuf(6,tri_pos_buf);
		r += vert_shader.dispatch(vec2(blocksize-1));
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		
		r += norm_shader.setStorageBuf(6,tri_pos_buf);
		r += norm_shader.setStorageBuf(7,tri_nrm_buf);
		r += norm_shader.dispatch(vec3(tris_count,1,1));
	}

	void draw(ShaderManager &) {
		DrawData dd;
		dd.positions.set<vec4>(tri_pos_buf);
		dd.normals.set<vec4>(tri_nrm_buf);

		shader.getCamera().set3D(vec2(640,480),vec3(0,0,5),vec3());
		shader.getModelStack().top(rotm);

		shader.draw(dd);
	}

	void rotate(vec2 d) {
		vec3 u = shader.getCamera().u();
		vec3 r = shader.getCamera().r();
		rotm = Quat(u,-d.x/80) * Quat(r,-d.y/80) * rotm;
	}

	void zoom(float am) {
		rotm = MATRIX::scaling(vec3(am)) * rotm;
	}

	void tess() {
		Clock clk;
		calc_density();
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); // yay

		calc_trin();
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); // yay

		calc_trioff_cpu();
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); // yay
		
		create_tri_poses();
		glFlush();

		cout << "tessellation took " << clk.getSeconds()*1000 << "ms" << endl;
		cout << "tris count " << tris_count << endl;
	}

	void set_time(Time t) {
		density_shader.setUniform("u_time",float(t.asSecs()));
		vert_shader.setUniform("u_time",float(t.asSecs()));
	}
};

class Widget : public GuiElement, public ClickListener, public ScrollListener
{
public:
	fm::Delegate<void,fg::ShaderManager &> ondraw; ///< Callback used when drawing
	fm::Delegate<void,fm::vec2,fm::vec2> onmousemove; ///< Callback used when mouse moves
	fm::Delegate<void,float> onscroll; ///< Callback used when scrolling happens
	fm::Delegate<void,fw::Event> onevent; ///< Callback used when any event happens
	fm::Delegate<void,fw::Keyboard::Key> onkeypress; ///< Callback used when a key is pressed
	fm::Delegate<void> onupdate; ///< Callback used once every frame

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

	/////////////////////////////////////////////////////////////
	/// @brief Handle an event
	/// 
	/// @param ev The event
	/// 
	/// @return True iff the event got handled
	/// 
	/////////////////////////////////////////////////////////////
	virtual bool onEvent(fw::Event &ev) override;

	/////////////////////////////////////////////////////////////
	/// @brief update the gui element
	/// 
	/// @param dt The elapsed time since last update
	/// 
	/////////////////////////////////////////////////////////////
	virtual void onUpdate(const fm::Time &dt) override;
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

/////////////////////////////////////////////////////////////
void Widget::onUpdate(const fm::Time &dt)
{
	GuiElement::onUpdate(dt);
	onupdate(dt);
}

/////////////////////////////////////////////////////////////
bool Widget::onEvent(fw::Event &ev)
{
	bool handled = GuiElement::onEvent(ev);
	onevent(ev);

	if (ev.type == Event::KeyPressed) {
		onkeypress(ev.key.code);
	}

	return handled;
}

int main()
{
	GuiWindow win(vec2(640,480));
	win.setClearColor(vec4::Black);
	win.setDepthTest(LEqual);
	cout << "init: ok" << endl;

	App app;
	app.tess();
	app.set_time(Time::Zero);

	bool realtime = false;
	Clock rtclk(true);

	Widget *w = new Widget(win,win.getSize());
	w->ondraw = [&](ShaderManager &shader) {
		app.draw(shader);
	};
	w->onmousemove = [&](vec2 a,vec2 b) {
		if (w->isPressed(Mouse::Left))
			app.rotate(a-b);
	};
	w->onscroll = [&](float d) {
		app.zoom(pow(1.3,d));
	};
	w->onevent = [&](Event ev) {
		if (ev.type == Event::FocusGained) {
			app.init_draw_shader();
		}
	};
	w->onkeypress = [&](Keyboard::Key key) {
		if (key == Keyboard::Plus) {
			app.mulblocksize(1.2);
		}
		if (key == Keyboard::Minus) {
			app.mulblocksize(1/1.2);
		}
		if (key == Keyboard::R) {
			app.init_shaders();
			rtclk.restart();
			app.tess();
		}
		if (key == Keyboard::Enter) {
			Image img = win.capture();
			for (int i=0;;++i) {
				stringstream ss;
				ss << "capture" << i << ".png";
				if (!ifstream(ss.str())) {
					img.saveToFile(ss.str());
					break;
				}
			}
		}
		if (key == Keyboard::T) {
			realtime = !realtime;
			rtclk.togglePause();
		}
		if (key == Keyboard::D) {
			app.dump_bufs();
		}
	};
	w->onupdate = [&]() {
		if (realtime) {
			app.set_time(rtclk.getTime());
			app.tess();
		}
	};
	win.getMainLayout().addChildElement(w);

	win.runGuiLoop();
}
