#pragma once

#include "gui/gui.h"
#include <SFML\Graphics.hpp>
#include <map>
#include <set>
#include <functional>

namespace ui {
	
	Element& root(){
		static Element* root { new FreeElement() };
		return *root->shared_this;
	}
	
	void addKeyboardCommand(Key trigger_key, std::function<void()> handler){
		getContext().addKeyboardCommand(trigger_key, handler);
	}
	
	void addKeyboardCommand(Key trigger_key, std::vector<Key> required_keys, std::function<void()> handler){
		getContext().addKeyboardCommand(trigger_key, required_keys, handler);
	}
	
	void setQuitHandler(std::function<bool()> handler){
		getContext().setQuitHandler(handler);
	}
	
	void startTransition(float duration, std::function<void(float)> transitionFn, std::function<void()> onComplete){
		getContext().addTransition(Transition(duration, transitionFn, onComplete));
	}
	
	double getProgramTime(){
		return getContext().getProgramTime();
	}
	
	vec2 getScreenSize(){
		sf::Vector2u size = getContext().getRenderWindow().getSize();
		return vec2((float)size.x, (float)size.y);
	}
	
	vec2 getMousePos(){
		return vec2(sf::Mouse::getPosition(getContext().getRenderWindow()));
	}
	
	Context& getContext(){
		static Context context;
		return context;
	}
	
	void init(unsigned width, unsigned height, std::string title, int target_fps){
		getContext().init(width, height, title, 1.0 / target_fps);
	}
	
	void quit(bool force){
		getContext().handleQuit(force);
	}
	
	void run(){
		double prev_time = getContext().getProgramTime();
		while (getContext().getRenderWindow().isOpen() && !getContext().hasQuit()){
			sf::Event event;
			while (getContext().getRenderWindow().pollEvent(event)){
				switch (event.type){
					case sf::Event::Closed:
						quit();
						break;
					case sf::Event::Resized:
						getContext().resize(event.size.width, event.size.height);
						break;
					case sf::Event::LostFocus:
						getContext().setDraggingElement({});
						break;
					case sf::Event::TextEntered:
						if (auto text_entry = getContext().getTextEntry()){
							if (event.text.unicode >= 32 && event.text.unicode < 127){
								text_entry->write(static_cast<char>(event.text.unicode));
							}
						}
						break;
					case sf::Event::KeyPressed:
						if (auto text_entry = getContext().getTextEntry()){
							switch (event.key.code){
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
									getContext().handleKeyPress(event.key.code);
									break;
							}
						} else {
							getContext().handleKeyPress(event.key.code);
						}
						
						break;
					case sf::Event::KeyReleased:
						getContext().handleKeyRelease(event.key.code);
						break;
					case sf::Event::MouseButtonPressed:
					{
						getContext().handleMouseDown(event.mouseButton.button,
													 vec2((float)event.mouseButton.x,
														  (float)event.mouseButton.y));
						break;
					}
					case sf::Event::MouseButtonReleased:
						getContext().handleMouseUp(event.mouseButton.button,
												   vec2((float)event.mouseButton.x,
														(float)event.mouseButton.y));
						break;
					case sf::Event::MouseWheelScrolled:
					{
						vec2 pos = {
							(float)event.mouseWheelScroll.x,
							(float)event.mouseWheelScroll.y
						};
						if (event.mouseWheelScroll.wheel == sf::Mouse::Wheel::VerticalWheel){
							getContext().handleScroll(pos, event.mouseWheelScroll.delta, 0.0f);
						} else if (event.mouseWheelScroll.wheel == sf::Mouse::Wheel::HorizontalWheel){
							getContext().handleScroll(pos, 0.0f, event.mouseWheelScroll.delta);
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
			root().update(root().getSize().x);

			// clear the screen
			getContext().getRenderWindow().clear();
			getContext().resetView();

			// render the root element, and all children it contains
			root().setSize(getScreenSize());
			root().renderChildren(getContext().getRenderWindow());

			// highlight current element if alt is pressed
			if (auto curr = getContext().getCurrentElement()){
				if ((sf::Keyboard::isKeyPressed(Key::LAlt) || sf::Keyboard::isKeyPressed(Key::RAlt))){
					sf::RectangleShape rect(curr->getSize());
					rect.setPosition(curr->rootPos());
					rect.setFillColor(sf::Color(0));
					rect.setOutlineColor(sf::Color(0xFFFF0080));
					rect.setOutlineThickness(2);
					getContext().getRenderWindow().draw(rect);
				}
			}

			getContext().getRenderWindow().display();

			// sleep only as long as needed
			double now = getContext().getProgramTime();
			double delay = getContext().getRenderDelay();
			sf::sleep(sf::seconds(std::max(0.0f, (float)(getContext().getRenderDelay() - (now - prev_time)))));
			prev_time = now;
		}

		root().clear();
	}
}