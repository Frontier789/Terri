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

	cam.set3D(vec2(640,480),vec3(0,0,5),vec3());

	vector<Block> blocks;
	for (int x=-3;x<3;++x) {
	for (int y=-3;y<3;++y) {
	for (int z=-3;z<3;++z) {
		blocks.emplace_back(factory.createBlock());
		blocks.back().set_offset(vec3(x*2,y*2,z*2));
	}}}

	map<Keyboard::Key,bool> key_pressed;

	Widget *w = new Widget(win,win.getSize());
	w->ondraw = [&](ShaderManager &shader) {
		for (auto &block : blocks) block.draw(shader);
	};
	w->onmousemove = [&](vec2 a,vec2 b) {
		if (w->isPressed(Mouse::Left)) {
			cam.addPitch(fm::deg(b.y-a.y)*.3);
			cam.addYaw(fm::deg(a.x-b.x)*.3);
		}
	};
	w->onscroll = [&](float d) {
		for (auto &block : blocks) block.zoom(pow(1.3,d));
	};
	w->onevent = [&](Event ev) {
		if (ev.type == Event::FocusGained) {
			factory.init_draw_shader();
		}
	};
	w->onkeypress = [&](Keyboard::Key key) {
		if (key == Keyboard::Plus) {
			for (auto &block : blocks) block.mulblocksize(1.2);
		}
		if (key == Keyboard::Minus) {
			for (auto &block : blocks) block.mulblocksize(1/1.2);
		}
		if (key == Keyboard::R) {
			factory.init_shaders();
			for (auto &block : blocks) {
				block.init_buffers();
				block.tess();
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
		cam.movePosition(dir * dt.s());
	};

	win.getMainLayout().addChildElement(w);

	win.runGuiLoop();
}