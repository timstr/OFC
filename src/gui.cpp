#pragma once

#include "gui/gui.h"
#include <SFML\Graphics.hpp>
#include <map>
#include <set>
#include <functional>

namespace ui {
	
	Window& root(){
		static std::shared_ptr<Window> rootwin { std::make_shared<Window>() };
		return *rootwin;
	}
	void addKeyboardCommand(sf::Keyboard::Key trigger_key, std::function<void()> handler){
		getContext().addKeyboardCommand(trigger_key, handler);
	}
	void addKeyboardCommand(sf::Keyboard::Key trigger_key, std::vector<sf::Keyboard::Key> required_keys, std::function<void()> handler){
		getContext().addKeyboardCommand(trigger_key, required_keys, handler);
	}
	void setQuitHandler(std::function<bool()> handler){
		getContext().setQuitHandler(handler);
	}
	void startTransition(double duration, std::function<void(double)> transitionFn, std::function<void()> onComplete){
		getContext().addTransition(Transition(duration, transitionFn, onComplete));
	}
	long double getProgramTime(){
		return getContext().getProgramTime();
	}
	vec2 getScreenSize(){
		sf::Vector2u size = getContext().getRenderWindow().getSize();
		return vec2((float)size.x, (float)size.y);
	}
	vec2 getMousePos(){
		return vec2(sf::Mouse::getPosition(getContext().getRenderWindow()));
	}
	Context & getContext(){
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
		long double prev_time = getContext().getProgramTime();
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
						getContext().setDraggingWindow({});
						break;
					case sf::Event::TextEntered:
						// TODO: this looks sketchy
						if (auto text_entry = getContext().getTextEntry().lock()){
							if (event.text.unicode != '\b'){
								text_entry->write(static_cast<char>(event.text.unicode));
							}
						}
						break;
					case sf::Event::KeyPressed:
						if (auto text_entry = getContext().getTextEntry().lock()){
							switch (event.key.code){
								case sf::Keyboard::BackSpace:
									text_entry->onBackspace();
									break;
								case sf::Keyboard::Delete:
									text_entry->onDelete();
									break;
								case sf::Keyboard::Left:
									text_entry->onLeft();
									break;
								case sf::Keyboard::Right:
									text_entry->onRight();
									break;
								case sf::Keyboard::Home:
									text_entry->onHome();
									break;
								case sf::Keyboard::End:
									text_entry->onEnd();
									break;
								case sf::Keyboard::Return:
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
						if (auto curr = getContext().getCurrentWindow().lock()){
							curr->onKeyUp(event.key.code);
						}
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
						if (auto curr = getContext().getCurrentWindow().lock()){
							if (event.mouseWheelScroll.wheel == sf::Mouse::Wheel::HorizontalWheel){
								curr->onScroll(event.mouseWheelScroll.delta, 0.0);
							} else if (event.mouseWheelScroll.wheel == sf::Mouse::Wheel::HorizontalWheel){
								curr->onScroll(0.0, event.mouseWheelScroll.delta);
							}
						}
						break;
				}
			}

			// cache current time
			getContext().updateTime();

			//drag what's being dragged
			getContext().handleDrag();

			//mouse-over what needs mousing over
			getContext().handleHover();

			//apply transitions
			getContext().applyTransitions();

			//clear the screen
			getContext().getRenderWindow().clear();
			getContext().resetView();

			//render the root window, and all child windows it contains
			root().size = getScreenSize();
			root().render(getContext().getRenderWindow());

			//highlight current window if alt is pressed
			if (auto curr = getContext().getCurrentWindow().lock()){
				if ((sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt) || sf::Keyboard::isKeyPressed(sf::Keyboard::RAlt))){
					sf::RectangleShape rect(curr->size);
					rect.setPosition(curr->rootPos());
					rect.setFillColor(sf::Color(0));
					rect.setOutlineColor(sf::Color(0xFFFF0080));
					rect.setOutlineThickness(2);
					getContext().getRenderWindow().draw(rect);
				}
			}

			getContext().getRenderWindow().display();

			//sleep only as long as needed
			long double now = getContext().getProgramTime();
			double delay = getContext().getRenderDelay();
			sf::sleep(sf::seconds(std::max(0.0f, (float)(getContext().getRenderDelay() - (now - prev_time)))));
			prev_time = now;
		}

		root().clear();
	}
}