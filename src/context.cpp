#include "gui/context.h"
#include "gui/gui.h"
#include <iostream>

namespace ui {

	// calls `function` on `element` and all its ancestors until one returns true, and that element is returned
	template<typename ...ArgsT>
	std::shared_ptr<Element> propagate(std::shared_ptr<Element> element, bool (Element::* function)(ArgsT...), ArgsT... args){
		while (element) {
			if (((*element).*function)(args...)){
				return element;
			}
			element = element->getParent().lock();
		}
		return nullptr;
	}

	Context::Context() : doubleclicktime(0.25f), quit(false), current_element(root().shared_this) {
	
	}
	
	void Context::init(unsigned width, unsigned height, std::string title, double _render_delay){
		render_delay = _render_delay;
		sf::ContextSettings settings;
		settings.antialiasingLevel = 8;
		getRenderWindow().create(sf::VideoMode(width, height), title, sf::Style::Default, settings);
		resetView();
		clock.restart();
	}
	
	void Context::addTransition(Transition transition){
		transitions.push_back(transition);
	}
	
	void Context::applyTransitions(){
		for (auto it = transitions.begin(); it != transitions.end();){
			if (it->complete()){
				it = transitions.erase(it);
			} else {
				it->apply();
				++it;
			}
		}
	}
	
	void Context::focusTo(std::shared_ptr<Element> element){
		if (element){
			if (current_element){
				// if the element is already in focus, no work to do here
				if (element == current_element){
					return;
				}

				// stop typing now
				if (!getTextEntry()){
					setTextEntry(nullptr);
				}

				// paths to old and new focused element
				std::vector<std::shared_ptr<Element>> oldpath, newpath;

				// populate old
				std::shared_ptr<Element> old = current_element;
				while (old){
					oldpath.push_back(old);
					old = old->parent.lock();
				}

				// populate new
				std::shared_ptr<Element> nu = element;
				while (nu){
					newpath.push_back(nu);
					nu = nu->parent.lock();
				}

				// remove common parts
				while (!oldpath.empty() && !newpath.empty() && (oldpath.back() == newpath.back())){
					oldpath.pop_back();
					newpath.pop_back();
				}

				// prevent redundent calls
				current_element = element;

				// call handlers in order
				for (auto it = oldpath.begin(); it != oldpath.end(); ++it){
					(*it)->onLoseFocus();
				}
				for (auto it = newpath.rbegin(); it != newpath.rend(); ++it){
					(*it)->onFocus();
				}
			} else {
				std::cerr << "Warning: The GUI Context's current element is invalid." << std::endl;
			}
		} else {
			std::cerr << "Warning: An invalid element was attempted to be focused to" << std::endl;
		}
	}
	
	vec2 Context::getMousePosition(){
		return (vec2)sf::Mouse::getPosition(renderwindow);
	}
	
	void Context::handleMouseDown(sf::Mouse::Button button, vec2 pos){
		std::shared_ptr<Element> hit_element = root().findElementAt(pos);

		if (!hit_element){
			return;
		}

		bool recent = (clock.getElapsedTime() - click_timestamp).asSeconds() <= doubleclicktime;

		bool same_button = click_button == button;

		bool same_element = false;
		if (click_button == sf::Mouse::Left){
			same_element = left_clicked_element == hit_element;
		} else if (click_button == sf::Mouse::Right){
			same_element = right_clicked_element == hit_element;
		}

		// with the same button, on the same element:
		if (recent && same_button && same_element){

			// double click that element
			if (button == sf::Mouse::Left){
				left_clicked_element = propagate(hit_element, &Element::onLeftClick, 2);
			} else if (button == sf::Mouse::Right){
				right_clicked_element = propagate(hit_element, &Element::onRightClick, 2);
			}

			// don't let it be double clicked again until after it gets single clicked again
			// achieved by faking an old timestamp
			click_timestamp = clock.getElapsedTime() - sf::seconds(doubleclicktime);
		} else {
			// otherwise, single click

			focusTo(hit_element);
			if (button == sf::Mouse::Left){
				left_clicked_element = propagate(hit_element, &Element::onLeftClick, 1);
			} else if (button == sf::Mouse::Right){
				right_clicked_element = propagate(hit_element, &Element::onRightClick, 1);
			}

			click_timestamp = clock.getElapsedTime();
		}

		click_button = button;
	}

	void Context::handleMouseUp(sf::Mouse::Button button){
		if (button == sf::Mouse::Left){
			if (left_clicked_element && !left_clicked_element->isClosed()){
				left_clicked_element->onLeftRelease();
			}
		} else if (button == sf::Mouse::Right){
			if (right_clicked_element && !right_clicked_element->isClosed()){
				right_clicked_element->onRightRelease();
			}
		}
	}
	
	void Context::addKeyboardCommand(Key trigger_key, std::function<void()> handler){
		auto pair = std::pair<Key, std::vector<Key>>(trigger_key, {});
		commands[pair] = handler;
	}
	
	void Context::addKeyboardCommand(Key trigger_key, std::vector<Key> required_keys, std::function<void()> handler){
		auto pair = std::pair<Key, std::vector<Key>>(trigger_key, required_keys);
		commands[pair] = handler;
	}
	
	void Context::setQuitHandler(std::function<bool()> handler){
		quit_handler = handler;
	}
	
	void Context::handleKeyPress(Key key){
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
		} else {
			auto elem = propagate(current_element, &Element::onKeyDown, key);
			auto it = keys_pressed.find(key);
			if (it != keys_pressed.end()){
				if (it->second != elem){
					it->second->onKeyUp(key);
					it->second = elem;
				}
			} else {
				keys_pressed[key] = elem;
			}
		}
	}

	void Context::handleKeyRelease(Key key){
		auto it = keys_pressed.find(key);
		if (it != keys_pressed.end()){
			if (it->second && !it->second->isClosed()){
				it->second->onKeyUp(key);
			}
			keys_pressed.erase(it);
		}
	}

	void Context::handleScroll(vec2 pos, float delta_x, float delta_y){
		auto hit_element = root().findElementAt(pos);
		propagate(hit_element, &Element::onScroll, delta_x, delta_y);
	}
	
	void Context::handleDrag(){
		if (dragging_element){
			dragging_element->pos = (vec2)sf::Mouse::getPosition(getRenderWindow()) - drag_offset;
			dragging_element->onDrag();
		}
	}
	
	void Context::handleHover(vec2 pos){
		auto element = root().findElementAt(pos, dragging_element);

		if (element != hover_element){
			// if the mouse is moved onto a new element
			
			std::vector<std::shared_ptr<Element>> oldpath, newpath;
			auto oldelem = hover_element;
			auto newelem = element;
			hover_element = element;

			while (oldelem){
				oldpath.push_back(oldelem);
				oldelem = oldelem->parent.lock();
			}

			while (newelem){
				newpath.push_back(newelem);
				newelem = newelem->parent.lock();
			}

			while (!oldpath.empty() && !newpath.empty() && oldpath.back() == newpath.back()){
				oldpath.pop_back();
				newpath.pop_back();
			}

			for (auto it = oldpath.begin(); it != oldpath.end(); ++it){
				(*it)->onMouseOut();
			}

			for (auto it = newpath.rbegin(); it != newpath.rend(); ++it){
				(*it)->onMouseOver();
			}
		}
		
		if (hover_element){
			if (dragging_element){
				propagate(hover_element, &Element::onHoverWith, dragging_element);
			} else {
				propagate(hover_element, &Element::onHover);
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
	
	std::shared_ptr<Element> Context::getDraggingElement() const {
		return dragging_element;
	}
	
	void Context::setDraggingElement(std::shared_ptr<Element> element, vec2 offset){
		dragging_element = element;
		drag_offset = offset;
	}
	
	std::shared_ptr<Element> Context::getCurrentElement() const{
		return current_element;
	}

	std::shared_ptr<Element> Context::getHoverElement() const {
		return hover_element;
	}
	
	std::shared_ptr<TextEntry> Context::getTextEntry() const {
		return text_entry;
	}
	
	void Context::setTextEntry(std::shared_ptr<TextEntry> textentry){
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