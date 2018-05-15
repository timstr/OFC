#include "gui/context.h"
#include "gui/gui.h"
#include <iostream>

namespace ui {

	Context::Context() : doubleclicktime(0.25f), quit(false) {
	
	}
	void Context::init(unsigned width, unsigned height, std::string title, double _render_delay){
		render_delay = _render_delay;
		sf::ContextSettings settings;
		settings.antialiasingLevel = 8;
		getRenderWindow().create(sf::VideoMode(width, height), title, sf::Style::Default, settings);
		resetView();
		current_window = root().weak_from_this();
		clock.restart();
	}
	void Context::addTransition(Transition transition){
		transitions.push_back(transition);
	}
	void Context::applyTransitions(){
		for (auto it = transitions.begin(); it != transitions.end(); ++it){
			if (it->complete()){
				it = transitions.erase(it);
			} else {
				it->apply();
			}
		}
	}
	void Context::focusTo(std::weak_ptr<Window> window){
		if (auto win = window.lock()){
			if (auto curr = current_window.lock()){
				// if the window is already in focus, no work to do here
				if (win == curr){
					return;
				}

				// stop typing now
				if (!getTextEntry().expired()){
					setTextEntry({});
				}

				// paths to old and new focused window
				std::vector<std::shared_ptr<Window>> oldpath, newpath;

				// populate old
				std::shared_ptr<Window> old = curr;
				while (old){
					oldpath.push_back(old);
					old = old->parent.lock();
				}

				// populate new
				std::shared_ptr<Window> nu = win;
				while (nu){
					newpath.push_back(nu);
					nu = nu->parent.lock();
				}

				// remove common parts
				while (!oldpath.empty() && !newpath.empty() && (oldpath.back() == newpath.back())){
					oldpath.pop_back();
					newpath.pop_back();
				}

				// call handlers in order
				for (auto it = oldpath.begin(); it != oldpath.end(); ++it){
					(*it)->onLoseFocus();
				}
				for (auto it = newpath.rbegin(); it != newpath.rend(); ++it){
					(*it)->onFocus();
				}
			} else {
				std::cerr << "Warning: The GUI Context's current window is invalid." << std::endl;
			}
		} else {
			std::cerr << "Warning: An invalid window was attempted to be focused to" << std::endl;
		}
	}
	vec2 Context::getMousePosition(){
		return (vec2)sf::Mouse::getPosition(renderwindow);
	}
	void Context::handleMouseDown(sf::Mouse::Button button, vec2 pos){
		std::shared_ptr<Window> hitwin = root().findWindowAt(pos).lock();

		if (!hitwin){
			return;
		}

		if ((clock.getElapsedTime() - click_timestamp).asSeconds() <= doubleclicktime){
			std::shared_ptr<Window> last_clicked = click_window.lock();

			// if the mouse has been clicked recently:
			if (click_button == button){
				// if the mouse was clicked with the same button:
				if (last_clicked && (hitwin == last_clicked)){
					// if the click hit the same window again:

					// double click that window
					if (button == sf::Mouse::Left){
						hitwin->onLeftClick(2);
					} else if (button == sf::Mouse::Right){
						hitwin->onRightClick(2);
					}

					// don't let it be double clicked again until after it gets single clicked again
					// achieved by faking an old timestamp
					click_timestamp = clock.getElapsedTime() - sf::seconds(doubleclicktime);
				} else {
					// if the click hit a different window:

					focusTo(hitwin);
					// single-click the first window, then this one
					if (button == sf::Mouse::Left){
						hitwin->onLeftClick(1);
					} else if (button == sf::Mouse::Right){
						hitwin->onRightClick(1);
					}
					click_timestamp = clock.getElapsedTime();
				}
			} else {
				// if the mouse was clicked with a different button:

				focusTo(hitwin);

				// single-click the current window
				if (button == sf::Mouse::Left){
					hitwin->onLeftClick(1);
				} else if (button == sf::Mouse::Right){
					hitwin->onRightClick(1);
				}

				click_timestamp = clock.getElapsedTime();
			}
		} else {
			// if the mouse hasn't been clicked recently:

			focusTo(hitwin);
			if (button == sf::Mouse::Left){
				hitwin->onLeftClick(1);
			} else if (button == sf::Mouse::Right){
				hitwin->onRightClick(1);
			}

			click_timestamp = clock.getElapsedTime();
		}

		click_button = button;
		click_window = hitwin;
	}
	void Context::addKeyboardCommand(sf::Keyboard::Key trigger_key, std::function<void()> handler){
		auto pair = std::pair<sf::Keyboard::Key, std::vector<sf::Keyboard::Key>>(trigger_key, {});
		commands[pair] = handler;
	}
	void Context::addKeyboardCommand(sf::Keyboard::Key trigger_key, std::vector<sf::Keyboard::Key> required_keys, std::function<void()> handler){
		auto pair = std::pair<sf::Keyboard::Key, std::vector<sf::Keyboard::Key>>(trigger_key, required_keys);
		commands[pair] = handler;
	}
	void Context::setQuitHandler(std::function<bool()> handler){
		quit_handler = handler;
	}
	void Context::handleKeyPress(sf::Keyboard::Key key){
		auto it = commands.begin();
		size_t max = 0;
		auto current_it = commands.end();
		while (it != commands.end()){
			if (it->first.first == key){
				bool match = true;
				for (int i = 0; i < it->first.second.size() && match; i++){
					match = sf::Keyboard::isKeyPressed(it->first.second[i]);
				}
				if (match && it->first.second.size() >= max){
					max = it->first.second.size();
					current_it = it;
				}
			}
			it++;
		}

		if (current_it != commands.end()){
			current_it->second();
		} else if (auto curr = current_window.lock()){
			curr->onKeyDown(key);
		}
	}
	void Context::handleMouseUp(sf::Mouse::Button button, vec2 pos){
		if (auto dragging = dragging_window.lock()){
			if (button == sf::Mouse::Left){
				dragging->onLeftRelease();
			} else if (button == sf::Mouse::Right){
				dragging->onRightRelease();
			}

			std::shared_ptr<Window> hover_window = root().findWindowAt(pos).lock();
			while (hover_window && !(hover_window->onDropWindow(dragging_window))){
				hover_window = hover_window->parent.lock();
			}
		} else if (auto curr = current_window.lock()){
			if (button == sf::Mouse::Left){
				curr->onLeftRelease();
			} else if (button == sf::Mouse::Right){
				curr->onRightRelease();
			}
		}
	}
	void Context::handleDrag(){
		if (auto dragging = dragging_window.lock()){
			dragging->pos = (vec2)sf::Mouse::getPosition(getRenderWindow()) - drag_offset;
			dragging->onDrag();
		}
	}
	void Context::handleHover(){
		vec2 position = (vec2)sf::Mouse::getPosition(getRenderWindow());
		std::shared_ptr<Window> hover_window = root().findWindowAt(position).lock();
		if (hover_window){
			if (auto draggin = dragging_window.lock()){
				hover_window->onHoverWithWindow(dragging_window);
			} else {
				hover_window->onHover();
			}
		}
	}
	void Context::handleQuit(bool force){
		if (quit_handler && !force){
			quit = quit_handler();
		} else {
			quit = true;
		}
	}
	bool Context::hasQuit(){
		return quit;
	}
	sf::RenderWindow& Context::getRenderWindow(){
		return renderwindow;
	}
	double Context::getRenderDelay(){
		return render_delay;
	}
	void Context::translateView(vec2 offset){
		view_offset.x -= offset.x;
		view_offset.y -= offset.y;
	}
	vec2 Context::getViewOffset(){
		return view_offset;
	}
	void Context::resetView(){
		vec2 size = getScreenSize();
		clip_rect = sf::FloatRect(0, 0, size.x, size.y);
		view_offset = vec2(0, 0);
		updateView();
	}
	const sf::FloatRect& Context::getClipRect(){
		return clip_rect;
	}
	void Context::setClipRect(const sf::FloatRect& rect){
		clip_rect = rect;
	}
	void Context::intersectClipRect(const sf::FloatRect& rect){
		float left = std::max(clip_rect.left, rect.left);
		float top = std::max(clip_rect.top, rect.top);
		float right = std::min(clip_rect.left + clip_rect.width, rect.left + rect.width);
		float bottom = std::min(clip_rect.top + clip_rect.height, rect.top + rect.height);
		clip_rect = sf::FloatRect(left, top, right - left, bottom - top);
	}
	void Context::resize(int w, int h){
		width = w;
		height = h;
	}
	void Context::updateTime(){
		program_time = clock.getElapsedTime().asMilliseconds() / 1000.0;
	}
	double Context::getProgramTime() const {
		return program_time;
	}
	std::weak_ptr<Window> Context::getDraggingWindow(){
		return dragging_window;
	}
	void Context::setDraggingWindow(std::weak_ptr<Window> window, vec2 offset){
		dragging_window = window;
		drag_offset = offset;
	}
	std::weak_ptr<Window> Context::getCurrentWindow(){
		return current_window;
	}
	std::weak_ptr<TextEntry> Context::getTextEntry(){
		return text_entry;
	}
	void Context::setTextEntry(std::weak_ptr<TextEntry> textentry){
		text_entry = textentry;
	}
	void Context::updateView(){
		vec2 size = getScreenSize();
		sf::View view;
		sf::FloatRect rect = getClipRect();
		vec2 offset = getViewOffset() + vec2(rect.left, rect.top);
		view.setSize(rect.width, rect.height);
		view.setCenter(offset.x + rect.width * 0.5f, offset.y + rect.height * 0.5f);
		sf::FloatRect vp = sf::FloatRect(
			rect.left / size.x,
			rect.top / size.y,
			rect.width / size.x,
			rect.height / size.y
		);
		view.setViewport(vp);
		renderwindow.setView(view);
	}

}