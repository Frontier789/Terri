#include <FRONTIER/OpenGL.hpp>
#include <Frontier.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

#include "BlockFactory.hpp"

using namespace std;

int main()
{
	GuiWindow win(vec2(640,480));
	win.setClearColor(vec4::Black);
	win.setDepthTest(LEqual);

	BlockFactory factory(1);
	factory.init();

	Camera &cam = factory.getCam();
	float flyspeed = 1;

	cam.set3D(vec2(640,480),vec3(0,0,5),vec3());

	auto sorter = [](vec3 a,vec3 b) {if (a.x != b.x) return a.x<b.x;if (a.y != b.y) return a.y<b.y;return a.z<b.z;};

	map<vec3i,Block*,decltype(sorter)> blocks(sorter);

	map<Keyboard::Key,bool> key_pressed;

	Widget *w = new Widget(win,win.getSize());
	w->ondraw = [&](ShaderManager &shader) {
		for (auto &block : blocks) block.second->draw(shader);
	};
	w->onmousemove = [&](vec2 a,vec2 b) {
		if (w->isPressed(Mouse::Left)) {
			cam.addPitch(fm::deg(b.y-a.y)*.3);
			cam.addYaw(fm::deg(a.x-b.x)*.3);
		}
	};
	w->onscroll = [&](float d) {
		for (auto &block : blocks) block.second->zoom(pow(1.3,d));
	};
	w->onevent = [&](Event ev) {
		if (ev.type == Event::FocusGained) {
			factory.init_draw_shader();
		}
	};
	w->onkeypress = [&](Keyboard::Key key) {
		if (key == Keyboard::Plus) {
			for (auto &block : blocks) block.second->mulblocksize(1.2);
		}
		if (key == Keyboard::Minus) {
			for (auto &block : blocks) block.second->mulblocksize(1/1.2);
		}
		if (key == Keyboard::Q) flyspeed /= 1.5;
		if (key == Keyboard::E) flyspeed *= 1.5;
		
		if (key == Keyboard::R) {
			factory.init_shaders();
			for (auto &block : blocks) {
				block.second->init_buffers();
				block.second->tess();
			}
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

		key_pressed[key] = true;
	};
	w->onkeyrelease = [&](Keyboard::Key key) {
		key_pressed[key] = false;
	};
	w->onupdate = [&](Time dt) {
		vec3 f = cam.f();
		vec3 r = cam.r();
		vec3 u = vec3(0,1,0);
		vec3 dir;
		for (auto key : key_pressed) {
			if (key.second) {
				vec3i d = Keyboard::keyToDelta(key.first);
				dir += f*d.y + r*d.x + u*d.z;
			}
		}
		cam.movePosition(dir * dt.s() * flyspeed);

		vec3 p = cam.getPosition();
		vec3i k = vec3i((p+vec3(1))/2);
		for (int x=-5;x<5;++x) 
		for (int y=-5;y<5;++y) 
		for (int z=-5;z<5;++z) {
			vec3i s = k + vec3i(x,y,z);
			if (blocks.find(s) == blocks.end()) {
				blocks[s] = new Block(std::move(factory.createBlock()));
				blocks[s]->set_offset(s*2);
				if (blocks[s]->get_tris_count()) {
					x = 5;
					y = 5;
					z = 5;
				}
			}
		}
	};

	win.getMainLayout().addChildElement(w);

	win.runGuiLoop();

	for (auto block : blocks)
		delete block.second;
}