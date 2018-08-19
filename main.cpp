#include <FRONTIER/OpenGL.hpp>
#include <Frontier.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

#include "Block.hpp"

using namespace std;

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

	Block block(2);
	block.tess();
	block.set_time(Time::Zero);

	bool realtime = false;
	Clock rtclk(true);

	Widget *w = new Widget(win,win.getSize());
	w->ondraw = [&](ShaderManager &shader) {
		block.draw(shader);
	};
	w->onmousemove = [&](vec2 a,vec2 b) {
		if (w->isPressed(Mouse::Left))
			block.rotate(a-b);
	};
	w->onscroll = [&](float d) {
		block.zoom(pow(1.3,d));
	};
	w->onevent = [&](Event ev) {
		if (ev.type == Event::FocusGained) {
			block.init_draw_shader();
		}
	};
	w->onkeypress = [&](Keyboard::Key key) {
		if (key == Keyboard::Plus) {
			block.mulblocksize(1.2);
		}
		if (key == Keyboard::Minus) {
			block.mulblocksize(1/1.2);
		}
		if (key == Keyboard::R) {
			block.init_shaders();
			rtclk.restart();
			block.tess();
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
			block.dump_bufs();
		}
	};
	w->onupdate = [&]() {
		if (realtime) {
			block.set_time(rtclk.getTime());
			block.tess();
		}
	};
	win.getMainLayout().addChildElement(w);

	win.runGuiLoop();
}
