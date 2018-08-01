#include "GUI/Context.hpp"
#include "GUI/GUI.hpp"
#include <iostream>

namespace ui {

	// calls `function` on `element` and all its ancestors until one returns true, and that element is returned
	template<typename ...ArgsT>
	Ref<Element> propagate(Ref<Element> element, bool (Element::* function)(ArgsT...), ArgsT... args) {
		while (element) {
			if (((*element).*function)(std::forward<ArgsT>(args)...)) {
				return element;
			}
			element = element->parent().lock();
		}
		return nullptr;
	}

	Context::Context() :
		quit(false),
		render_delay(1.0f / 30.0f),
		doubleclicktime(0.25f),
		current_element(root().m_sharedthis) {

		program_time = clock.getElapsedTime().asSeconds();
		highlight_timestamp = clock.getElapsedTime() - sf::seconds(10.0f);
		click_timestamp = clock.getElapsedTime() - sf::seconds(10.0f);
	}

	void Context::init(unsigned _width, unsigned _height, std::string title, float _render_delay) {
		render_delay = _render_delay;
		sf::ContextSettings settings;
		settings.antialiasingLevel = 8;
		getRenderWindow().create(sf::VideoMode(_width, _height), title, sf::Style::Default, settings);
		resetView();
		clock.restart();
	}

	void Context::addTransition(Transition transition) {
		transitions.push_back(transition);
	}

	void Context::applyTransitions() {
		for (auto it = transitions.begin(); it != transitions.end();) {
			if (it->complete()) {
				it = transitions.erase(it);
			} else {
				it->apply();
				++it;
			}
		}
	}

	void Context::focusTo(Ref<Element> element) {
		if (element) {
			if (current_element) {
				// if the element is already in focus, no work to do here
				if (element == current_element) {
					return;
				}

				// stop typing now
				if (!getTextEntry()) {
					setTextEntry(nullptr);
				}

				// paths to old and new focused element
				std::vector<Ref<Element>> oldpath, newpath;

				// populate old
				Ref<Element> old = current_element;
				while (old) {
					oldpath.push_back(old);
					old = old->parent().lock();
				}

				// populate new
				Ref<Element> nu = element;
				while (nu) {
					newpath.push_back(nu);
					nu = nu->parent().lock();
				}

				// remove common parts
				while (!oldpath.empty() && !newpath.empty() && (oldpath.back() == newpath.back())) {
					oldpath.pop_back();
					newpath.pop_back();
				}

				// prevent redundent calls
				current_element = element;

				// call handlers in order
				for (auto it = oldpath.begin(); it != oldpath.end(); ++it) {
					(*it)->onLoseFocus();
				}
				for (auto it = newpath.rbegin(); it != newpath.rend(); ++it) {
					(*it)->onFocus();
				}
			} else {
				std::cerr << "Warning: The GUI Context's current element is invalid." << std::endl;
			}
		} else {
			std::cerr << "Warning: An invalid element was attempted to be focused to" << std::endl;
		}
	}

	vec2 Context::getMousePosition() {
		return (vec2)sf::Mouse::getPosition(renderwindow);
	}

	void Context::handleMouseDown(sf::Mouse::Button button, vec2 pos) {
		Ref<Element> hit_element = root().findElementAt(pos);

		if (!hit_element) {
			return;
		}

		bool recent = (clock.getElapsedTime() - click_timestamp).asSeconds() <= doubleclicktime;

		bool same_button = click_button == button;

		bool same_element = false;
		switch (click_button) {
			case sf::Mouse::Left:
				same_element = left_clicked_element == hit_element;
				break;
			case sf::Mouse::Right:
				same_element = right_clicked_element == hit_element;
				break;
			case sf::Mouse::Middle:
				same_element = middle_clicked_element == hit_element;
				break;
		}

		// with the same button, on the same element:
		if (recent && same_button && same_element) {

			// double click that element
			if (button == sf::Mouse::Left) {
				left_clicked_element = propagate(hit_element, &Element::onLeftClick, 2);
			} else if (button == sf::Mouse::Right) {
				right_clicked_element = propagate(hit_element, &Element::onRightClick, 2);
			} else if (button == sf::Mouse::Middle) {
				middle_clicked_element = propagate(hit_element, &Element::onMiddleClick, 2);
			}

			// don't let it be double clicked again until after it gets single clicked again
			// achieved by faking an old timestamp
			click_timestamp = clock.getElapsedTime() - sf::seconds(doubleclicktime);
		} else {
			// otherwise, single click

			focusTo(hit_element);
			if (button == sf::Mouse::Left) {
				left_clicked_element = propagate(hit_element, &Element::onLeftClick, 1);
			} else if (button == sf::Mouse::Right) {
				right_clicked_element = propagate(hit_element, &Element::onRightClick, 1);
			} else if (button == sf::Mouse::Middle) {
				middle_clicked_element = propagate(hit_element, &Element::onMiddleClick, 1);
			}

			click_timestamp = clock.getElapsedTime();
		}

		click_button = button;
	}

	void Context::handleMouseUp(sf::Mouse::Button button) {
		if (button == sf::Mouse::Left) {
			if (left_clicked_element && !left_clicked_element->isClosed()) {
				left_clicked_element->onLeftRelease();
			}
		} else if (button == sf::Mouse::Right) {
			if (right_clicked_element && !right_clicked_element->isClosed()) {
				right_clicked_element->onRightRelease();
			}
		} else if (button == sf::Mouse::Middle) {
			if (middle_clicked_element && !middle_clicked_element->isClosed()) {
				middle_clicked_element->onRightRelease();
			}
		}
	}

	void Context::releaseAllButtons() {
		// release all held keys
		for (const auto& key_elem : keys_pressed) {
			if (key_elem.second) {
				key_elem.second->onKeyUp(key_elem.first);
			}
		}
		keys_pressed.clear();

		// release left mouse button
		if (left_clicked_element) {
			left_clicked_element->onLeftRelease();
			left_clicked_element = nullptr;
		}

		// release right mouse button
		if (right_clicked_element) {
			right_clicked_element->onRightRelease();
			right_clicked_element = nullptr;
		}

		// release middle mouse button
		if (middle_clicked_element) {
			middle_clicked_element->onMiddleRelease();
			middle_clicked_element = nullptr;
		}
	}

	void Context::addKeyboardCommand(Key trigger_key, std::function<void()> handler) {
		auto pair = std::pair<Key, std::vector<Key>>(trigger_key, {});
		commands[pair] = handler;
	}

	void Context::addKeyboardCommand(Key trigger_key, std::vector<Key> required_keys, std::function<void()> handler) {
		auto pair = std::pair<Key, std::vector<Key>>(trigger_key, required_keys);
		commands[pair] = handler;
	}

	void Context::setQuitHandler(std::function<bool()> handler) {
		quit_handler = handler;
	}

	void Context::handleKeyDown(Key key) {
		// search for longest matching set of keys in registered commands
		size_t max = 0;
		auto current_cmd = commands.end();
		for (auto cmd_it = commands.begin(), end = commands.end(); cmd_it != end; ++cmd_it) {
			if (cmd_it->first.first == key) {
				bool match = true;
				for (int i = 0; i < cmd_it->first.second.size() && match; i++) {
					match = sf::Keyboard::isKeyPressed(cmd_it->first.second[i]);
				}
				if (match && cmd_it->first.second.size() >= max) {
					max = cmd_it->first.second.size();
					current_cmd = cmd_it;
				}
			}
		}

		if (current_cmd != commands.end()) {
			// if one was found, invoke that command
			current_cmd->second();
			return;
		}

		// if no command was found, send key stroke to the current element
		auto elem = propagate(current_element, &Element::onKeyDown, key);
		// and send key up to last element receiving same keystroke
		// in case of switching focus while key is held
		auto key_it = keys_pressed.find(key);
		if (key_it != keys_pressed.end()) {
			if (key_it->second && key_it->second != elem) {
				key_it->second->onKeyUp(key);
				key_it->second = elem;
			}
		} else if (elem) {
			keys_pressed[key] = elem;
		}

		// keyboard navigation
		auto parent = current_element->parent().lock();
		if (parent && parent->keyboardNavigable()) {
			if (key == ui::Key::Tab && (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift))) {
				// navigate to previous element
				if (current_element->navigateToPreviousElement()) {
					return;
				}
			} else if (key == ui::Key::Tab) {
				// navigate to next element
				if (current_element->navigateToNextElement()) {
					return;
				}
			} else if (key == ui::Key::Return) {
				// navigate in
				if (current_element->navigateIn()) {
					return;
				}
			} else if (key == ui::Key::Escape) {
				// navigate out
				if (current_element->navigateOut()) {
					return;
				}
				highlightCurrentElement();
			}
		}
	}

	void Context::handleKeyUp(Key key) {
		auto it = keys_pressed.find(key);
		if (it != keys_pressed.end()) {
			if (it->second && !it->second->isClosed()) {
				it->second->onKeyUp(key);
			}
			keys_pressed.erase(it);
		}
	}

	void Context::handleScroll(vec2 pos, float delta_x, float delta_y) {
		auto hit_element = root().findElementAt(pos);
		propagate(hit_element, &Element::onScroll, delta_x, delta_y);
	}

	void Context::handleDrag() {
		if (dragging_element) {
			dragging_element->m_pos = (vec2)sf::Mouse::getPosition(getRenderWindow()) - drag_offset;
			dragging_element->onDrag();
		}
	}

	void Context::handleHover(vec2 pos) {
		auto element = root().findElementAt(pos, dragging_element);

		if (element != hover_element) {
			// if the mouse is moved onto a new element

			std::vector<Ref<Element>> oldpath, newpath;
			auto oldelem = hover_element;
			auto newelem = element;
			hover_element = element;

			while (oldelem) {
				oldpath.push_back(oldelem);
				oldelem = oldelem->parent().lock();
			}

			while (newelem) {
				newpath.push_back(newelem);
				newelem = newelem->parent().lock();
			}

			while (!oldpath.empty() && !newpath.empty() && oldpath.back() == newpath.back()) {
				oldpath.pop_back();
				newpath.pop_back();
			}

			for (auto it = oldpath.begin(); it != oldpath.end(); ++it) {
				(*it)->onMouseOut();
			}

			for (auto it = newpath.rbegin(); it != newpath.rend(); ++it) {
				(*it)->onMouseOver();
			}
		}

		if (hover_element) {
			if (dragging_element) {
				propagate(hover_element, &Element::onHoverWith, dragging_element);
			} else {
				propagate(hover_element, &Element::onHover);
			}
		}
	}

	void Context::handleQuit(bool force) {
		if (quit_handler && !force) {
			quit = quit_handler();
		} else {
			quit = true;
		}
	}

	bool Context::hasQuit() {
		return quit;
	}

	sf::RenderWindow& Context::getRenderWindow() {
		return renderwindow;
	}

	float Context::getRenderDelay() {
		return render_delay;
	}

	void Context::setViewOffset(vec2 offset) {
		view_offset = offset;
	}

	vec2 Context::getViewOffset() {
		return view_offset;
	}

	void Context::resetView() {
		vec2 size = getScreenSize();
		clip_rect = sf::FloatRect(0, 0, size.x, size.y);
		view_offset = vec2(0, 0);
		updateView();
	}

	const sf::FloatRect& Context::getClipRect() {
		return clip_rect;
	}

	void Context::setClipRect(const sf::FloatRect& rect) {
		clip_rect = rect;
	}

	void Context::intersectClipRect(const sf::FloatRect& rect) {
		float left = std::max(clip_rect.left, rect.left);
		float top = std::max(clip_rect.top, rect.top);
		float right = std::min(clip_rect.left + clip_rect.width, rect.left + rect.width);
		float bottom = std::min(clip_rect.top + clip_rect.height, rect.top + rect.height);
		clip_rect = sf::FloatRect(left, top, right - left, bottom - top);
	}

	void Context::resize(int w, int h) {
		width = w;
		height = h;
	}

	void Context::updateTime() {
		program_time = clock.getElapsedTime().asSeconds();
	}

	float Context::getProgramTime() const {
		return program_time;
	}

	Ref<Element> Context::getDraggingElement() const {
		return dragging_element;
	}

	void Context::setDraggingElement(Ref<Element> element, vec2 offset) {
		dragging_element = element;
		drag_offset = offset;
	}

	Ref<Element> Context::getCurrentElement() const {
		return current_element;
	}

	Ref<Element> Context::getHoverElement() const {
		return hover_element;
	}

	Ref<TextEntry> Context::getTextEntry() const {
		return text_entry;
	}

	void Context::setTextEntry(Ref<TextEntry> textentry) {
		text_entry = textentry;
	}

	void Context::highlightCurrentElement() {
		highlight_timestamp = clock.getElapsedTime();
	}

	float Context::timeSinceHighlight() const {
		return program_time - highlight_timestamp.asSeconds();
	}

	void Context::updateView() {
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

	Context& getContext() {
		static Context context;
		return context;
	}

}