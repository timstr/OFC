#include "gui/context.h"
#include "gui/gui.h"
#include <iostream>

namespace ui {

	template<typename ...Args>
	void propagate(std::shared_ptr<Element> element, bool (Element::* function)(Args...), Args... args){
		while (element) {
			if (((*element).*function)(args...)){
				return;
			}
			element = element->getParent().lock();
		}
	}

	Context::Context() : doubleclicktime(0.25f), quit(false) {
	
	}
	
	void Context::init(unsigned width, unsigned height, std::string title, double _render_delay){
		render_delay = _render_delay;
		sf::ContextSettings settings;
		settings.antialiasingLevel = 8;
		getRenderWindow().create(sf::VideoMode(width, height), title, sf::Style::Default, settings);
		resetView();
		current_element = root().shared_from_this();
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

				current_element = element;

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

		// if the mouse has been clicked recently, with the same button, on the same element:
		if (((clock.getElapsedTime() - click_timestamp).asSeconds() <= doubleclicktime)
			&& (click_button == button)
			&& (clicked_element && (hit_element == clicked_element))){

			// double click that element
			if (button == sf::Mouse::Left){
				propagate(hit_element, &Element::onLeftClick, 2);
			} else if (button == sf::Mouse::Right){
				propagate(hit_element, &Element::onRightClick, 2);
			}

			// don't let it be double clicked again until after it gets single clicked again
			// achieved by faking an old timestamp
			click_timestamp = clock.getElapsedTime() - sf::seconds(doubleclicktime);
		} else {
			// otherwise, single click

			focusTo(hit_element);
			if (button == sf::Mouse::Left){
				propagate(hit_element, &Element::onLeftClick, 1);
			} else if (button == sf::Mouse::Right){
				propagate(hit_element, &Element::onRightClick, 1);
			}

			click_timestamp = clock.getElapsedTime();
		}

		click_button = button;
		clicked_element = hit_element;
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
			propagate(current_element, &Element::onKeyDown, key);
		}
	}

	void Context::handleKeyRelease(Key key){
		propagate(current_element, &Element::onKeyUp, key);
	}
	
	void Context::handleMouseUp(sf::Mouse::Button button, vec2 pos){
		if (button == sf::Mouse::Left){
			propagate(current_element, &Element::onLeftRelease);
		} else if (button == sf::Mouse::Right){
			propagate(current_element, &Element::onRightRelease);
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
			if (hover_element){
				propagate(hover_element, &Element::onMouseOut);
			}
			hover_element = element;
			if (hover_element){
				propagate(hover_element, &Element::onMouseOver);
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
	
	std::shared_ptr<Element> Context::getDraggingElement(){
		return dragging_element;
	}
	
	void Context::setDraggingElement(std::shared_ptr<Element> element, vec2 offset){
		dragging_element = element;
		drag_offset = offset;
	}
	
	std::shared_ptr<Element> Context::getCurrentElement(){
		return current_element;
	}
	
	std::shared_ptr<TextEntry> Context::getTextEntry(){
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