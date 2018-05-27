#pragma once

#include "element.h"
#include "transition.h"
#include "textentry.h"
#include <map>

namespace ui {

	struct Context {
		Context();

		void init(unsigned width, unsigned height, std::string title, double _render_delay);

		void addTransition(Transition transition);
		void applyTransitions();

		void addKeyboardCommand(Key trigger_key, std::function<void()> handler);
		void addKeyboardCommand(Key trigger_key, std::vector<Key> required_keys, std::function<void()> handler);
		void setQuitHandler(std::function<bool()> handler);

		void handleKeyPress(Key key);
		void handleMouseDown(sf::Mouse::Button button, vec2 pos);
		void handleMouseUp(sf::Mouse::Button button, vec2 pos);
		void handleDrag();
		void handleHover();
		void handleQuit(bool force);

		bool hasQuit();
		vec2 getMousePosition();
		sf::RenderWindow& getRenderWindow();
		double getRenderDelay();
		void translateView(vec2 offset);
		vec2 getViewOffset();
		void resetView();
		void updateView();
		const sf::FloatRect& getClipRect();
		void setClipRect(const sf::FloatRect& rect);
		void intersectClipRect(const sf::FloatRect& rect);
		void resize(int w, int h);

		void updateTime();
		double getProgramTime() const;

		std::weak_ptr<Element> getDraggingElement();
		void setDraggingElement(std::weak_ptr<Element> element, vec2 offset = vec2(0, 0));
		void focusTo(std::weak_ptr<Element> element);
		std::weak_ptr<Element> getCurrentElement();
		std::weak_ptr<TextEntry> getTextEntry();
		void setTextEntry(std::weak_ptr<TextEntry> textentry);

		private:

		// run() returns when quit is true
		bool quit;
		// desired time between frames, in seconds
		double render_delay;

		// cached current program time
		double program_time;

		// the renderwindow to which all ui elements are drawn
		sf::RenderWindow renderwindow;

		// the element currently being dragged
		std::weak_ptr<Element> dragging_element;
		// the mouse's relative position while dragging
		vec2 drag_offset;

		// the element currently in focus
		std::weak_ptr<Element> current_element;

		// the text entry currently being typed into
		std::weak_ptr<TextEntry> text_entry;

		// the element that was last clicked
		std::weak_ptr<Element> clicked_element;
		// maximum time between clicks of a double-click, in seconds
		const float doubleclicktime;
		// time of last click
		sf::Time click_timestamp;
		// the mouse button that was last clicked
		sf::Mouse::Button click_button;

		// active transitions
		std::vector<Transition> transitions;

		sf::Clock clock;

		// set of callback functions to be called during keystroke patterns
		std::map<std::pair<Key, std::vector<Key>>, std::function<void()>> commands;

		// callback function to be called when program is being closed, program continues if false is returned
		std::function<bool()> quit_handler;

		// portion of the screen currently being rendered to
		sf::FloatRect clip_rect;

		// translation of things being rendered
		vec2 view_offset;

		// width of the program's window
		int width;
		// height of the program's window
		int height;
	};
}