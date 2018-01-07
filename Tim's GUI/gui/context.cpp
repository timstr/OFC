#include "context.h"
#include "gui.h"

namespace ui {

	Context::Context() : doubleclicktime(0.25f) {
		quit = false;
		dragging_window = nullptr;
		current_window = nullptr;
		text_entry = nullptr;
	}
	void Context::init(unsigned width, unsigned height, std::string title, double _render_delay){
		render_delay = _render_delay;
		sf::ContextSettings settings;
		settings.antialiasingLevel = 8;
		getRenderWindow().create(sf::VideoMode(width, height), title, sf::Style::Default, settings);
		resetView();
		current_window = root();
		clock.restart();
	}
	void Context::addTransition(const Transition& transition){
		transitions.push_back(transition);
	}
	void Context::applyTransitions(){
		for (int i = 0; i < transitions.size();){
			if (transitions[i].complete()){
				transitions.erase(transitions.begin() + i);
			} else {
				transitions[i].apply();
				i++;
			}
		}
	}
	void Context::clearTransitions(Window* target){
		for (auto it = transitions.begin(); it != transitions.end();){
			if (it->target == target){
				transitions.erase(it);
			} else {
				it++;
			}
		}
	}
	void Context::focusTo(Window* window){
		if (current_window == window){
			return;
		}

		if (dragging_window){
			dragging_window = nullptr;
		}
		if (getTextEntry()){
			setTextEntry(nullptr);
		}

		std::vector<Window*> current_path;
		std::vector<Window*> new_path;

		Window* twindow = current_window;
		current_window = window;

		while (twindow != nullptr){
			current_path.push_back(twindow);
			twindow = twindow->parent;
		}

		twindow = window;
		while (twindow != nullptr){
			new_path.push_back(twindow);
			twindow = twindow->parent;
		}

		while ((current_path.size() > 0) && (new_path.size() > 0) && (current_path.back() == new_path.back())){
			current_path.pop_back();
			new_path.pop_back();
		}

		while (current_path.size() > 0){
			current_path.front()->onLoseFocus();
			current_path.erase(current_path.begin());
		}

		while (new_path.size() > 0){
			if (new_path.back()->parent && new_path.back()->bring_to_front){
				new_path.back()->bringToFront();
			}
			new_path.back()->onFocus();
			new_path.pop_back();
		}
	}
	vec2 Context::getMousePosition(){
		return (vec2)sf::Mouse::getPosition(renderwindow);
	}
	void Context::handleMouseDown(sf::Mouse::Button button, vec2 pos){
		Window* hitwin = root()->findWindowAt(pos);

		if (hitwin == nullptr){
			return;
		}

		if ((clock.getElapsedTime() - click_timestamp).asSeconds() <= doubleclicktime){
			// if the mouse has been clicked recently:
			if (click_button == button){
				// if the mouse was clicked with the same button:
				if (hitwin == click_window){
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
	void Context::addKeyboardCommand(sf::Keyboard::Key trigger_key, const std::function<void()>& handler){
		auto pair = std::pair<sf::Keyboard::Key, std::vector<sf::Keyboard::Key>>(trigger_key, {});
		auto it = commands.find(pair);
		if (it == commands.end()){
			commands[pair] = handler;
		} else {
			throw;
		}
	}
	void Context::addKeyboardCommand(sf::Keyboard::Key trigger_key, const std::vector<sf::Keyboard::Key>& required_keys, const std::function<void()>& handler){
		auto pair = std::pair<sf::Keyboard::Key, std::vector<sf::Keyboard::Key>>(trigger_key, required_keys);
		auto it = commands.find(pair);
		if (it == commands.end()){
			commands[pair] = handler;
		} else {
			throw;
		}
	}
	void Context::setQuitHandler(const std::function<bool()>& handler){
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
		} else if (current_window){
			current_window->onKeyDown(key);
		}
	}
	void Context::handleMouseUp(sf::Mouse::Button button, vec2 pos){
		if (dragging_window){
			if (button == sf::Mouse::Left){
				dragging_window->onLeftRelease();
			} else if (button == sf::Mouse::Right){
				dragging_window->onRightRelease();
			}

			Window* hover_window = root()->findWindowAt(pos);
			while (hover_window && !(hover_window->onDropWindow(dragging_window))){
				hover_window = hover_window->parent;
			}
			dragging_window = nullptr;
		} else if (current_window){
			if (button == sf::Mouse::Left){
				current_window->onLeftRelease();
			} else if (button == sf::Mouse::Right){
				current_window->onRightRelease();
			}
		}
	}
	void Context::handleDrag(){
		if (dragging_window){
			dragging_window->pos = (vec2)sf::Mouse::getPosition(getRenderWindow()) - drag_offset;
			dragging_window->onDrag();
		}
	}
	void Context::handleHover(){
		Window* hover_window = root()->findWindowAt((vec2)sf::Mouse::getPosition(getRenderWindow()));
		if (hover_window){
			if (dragging_window){
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
	long double Context::getProgramTime(){
		return clock.getElapsedTime().asMilliseconds() / 1000.0;
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
	Window* Context::getDraggingWindow(){
		return dragging_window;
	}
	void Context::setDraggingWindow(Window* window, vec2 offset){
		dragging_window = window;
		drag_offset = offset;
	}
	Window* Context::getCurrentWindow(){
		return current_window;
	}
	TextEntry* Context::getTextEntry(){
		return text_entry;
	}
	void Context::setTextEntry(TextEntry* textentry){
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