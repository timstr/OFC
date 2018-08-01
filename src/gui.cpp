#pragma once

#include "GUI/GUI.hpp"
#include <SFML/Graphics.hpp>
#include <map>
#include <set>
#include <functional>

namespace ui {

	namespace {
		Ref<Element> root_ptr;
	}

	Element& root() {
		static bool initialized = false;
		if (!initialized) {
			Element* rawroot = new FreeElement();
			root_ptr = rawroot->shared_from_this();
			initialized = true;
		}
		return *root_ptr;
	}

	void addKeyboardCommand(Key trigger_key, std::function<void()> handler) {
		getContext().addKeyboardCommand(trigger_key, handler);
	}

	void addKeyboardCommand(Key trigger_key, std::vector<Key> required_keys, std::function<void()> handler) {
		getContext().addKeyboardCommand(trigger_key, required_keys, handler);
	}

	void setQuitHandler(std::function<bool()> handler) {
		getContext().setQuitHandler(handler);
	}

	void startTransition(float duration, std::function<void(float)> transitionFn, std::function<void()> onComplete) {
		getContext().addTransition(Transition(duration, transitionFn, onComplete));
	}

	float getProgramTime() {
		return getContext().getProgramTime();
	}

	vec2 getScreenSize() {
		sf::Vector2u size = getContext().getRenderWindow().getSize();
		return vec2((float)size.x, (float)size.y);
	}

	vec2 getMousePos() {
		return vec2(sf::Mouse::getPosition(getContext().getRenderWindow()));
	}

	void init(unsigned width, unsigned height, std::string title, int target_fps) {
		getContext().init(width, height, title, 1.0f / target_fps);
	}

	void quit(bool force) {
		getContext().handleQuit(force);
	}

	void run() {
		float prev_time = getContext().getProgramTime();
		while (getContext().getRenderWindow().isOpen() && !getContext().hasQuit()) {
			sf::Event event;
			while (getContext().getRenderWindow().pollEvent(event)) {
				switch (event.type) {
					case sf::Event::Closed:
						quit();
						break;
					case sf::Event::Resized:
						getContext().resize(event.size.width, event.size.height);
						break;
					case sf::Event::LostFocus:
						getContext().setDraggingElement(nullptr);
						getContext().releaseAllButtons();
						break;
					case sf::Event::TextEntered:
						if (auto text_entry = getContext().getTextEntry()) {
							if (event.text.unicode >= 32 && event.text.unicode < 127) {
								text_entry->write(static_cast<char>(event.text.unicode));
							}
						}
						break;
					case sf::Event::KeyPressed:
						if (auto text_entry = getContext().getTextEntry()) {
							switch (event.key.code) {
								case Key::BackSpace:
									text_entry->onBackspace();
									break;
								case Key::Delete:
									text_entry->onDelete();
									break;
								case Key::Left:
									text_entry->onLeft();
									break;
								case Key::Right:
									text_entry->onRight();
									break;
								case Key::Home:
									text_entry->onHome();
									break;
								case Key::End:
									text_entry->onEnd();
									break;
								case Key::Return:
									text_entry->onReturn(text_entry->getText());
									break;
								default:
									getContext().handleKeyDown(event.key.code);
									break;
							}
						} else {
							getContext().handleKeyDown(event.key.code);
						}

						break;
					case sf::Event::KeyReleased:
						getContext().handleKeyUp(event.key.code);
						break;
					case sf::Event::MouseButtonPressed:
					{
						getContext().handleMouseDown(event.mouseButton.button,
													 vec2((float)event.mouseButton.x,
													 (float)event.mouseButton.y));
						break;
					}
					case sf::Event::MouseButtonReleased:
						getContext().handleMouseUp(event.mouseButton.button);
						break;
					case sf::Event::MouseWheelScrolled:
					{
						vec2 pos = {
							(float)event.mouseWheelScroll.x,
							(float)event.mouseWheelScroll.y
						};
						if (event.mouseWheelScroll.wheel == sf::Mouse::Wheel::VerticalWheel) {
							getContext().handleScroll(pos, 0.0f, event.mouseWheelScroll.delta);
						} else if (event.mouseWheelScroll.wheel == sf::Mouse::Wheel::HorizontalWheel) {
							getContext().handleScroll(pos, event.mouseWheelScroll.delta, 0.0f);
						}
					}
					break;
				}
			}

			// cache current time
			getContext().updateTime();

			// drag what's being dragged
			getContext().handleDrag();

			//mouse-over what needs mousing over
			getContext().handleHover(getMousePos());

			// apply transitions
			getContext().applyTransitions();

			// update elements
			root().setSize(getScreenSize(), true);
			root().update(root().width());

			// clear the screen
			getContext().getRenderWindow().clear();
			getContext().resetView();

			// render the root element, and all children it contains
			root().renderChildren(getContext().getRenderWindow());

			// highlight current element if alt is pressed
			if ((sf::Keyboard::isKeyPressed(Key::LAlt) || sf::Keyboard::isKeyPressed(Key::RAlt))) {
				getContext().highlightCurrentElement();
			}

			// draw highlight around highlighted element
			if (auto curr = getContext().getCurrentElement()) {
				float elapsed = getContext().timeSinceHighlight();

				if (elapsed <= 2.0f) {
					float value = exp(-elapsed * 3.453877f);
					sf::Color color { 0xFFFF00FF };
					color.a = (uint8_t)(std::min(value, 1.0f) * 255.0f);

					sf::RectangleShape rect(curr->size());
					rect.setPosition(curr->absPos());
					rect.setFillColor(sf::Color(0));
					rect.setOutlineColor(color);
					rect.setOutlineThickness(2);
					getContext().getRenderWindow().draw(rect);
				}
			}

			getContext().getRenderWindow().display();

			// sleep only as long as needed
			float now = getContext().getProgramTime();
			float delay = getContext().getRenderDelay();
			sf::sleep(sf::seconds(std::max(0.0f, delay - now + prev_time)));
			prev_time = now;
		}

		// remove all windows from and close root
		root().close();

		// and allow root to be destroyed
		root_ptr = nullptr;
	}
}