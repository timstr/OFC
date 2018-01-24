#pragma once

#include "window.h"
#include "transition.h"
#include "textentry.h"
#include <map>

namespace ui {

	struct Context {
		Context();

		void init(unsigned width, unsigned height, std::string title, double _render_delay);

		void addTransition(Transition transition);
		void applyTransitions();
		void clearTransitions(Window* target);

		void addKeyboardCommand(sf::Keyboard::Key trigger_key, std::function<void()> handler);
		void addKeyboardCommand(sf::Keyboard::Key trigger_key, std::vector<sf::Keyboard::Key> required_keys, std::function<void()> handler);
		void setQuitHandler(std::function<bool()> handler);

		void handleKeyPress(sf::Keyboard::Key key);
		void handleMouseDown(sf::Mouse::Button button, vec2 pos);
		void handleMouseUp(sf::Mouse::Button button, vec2 pos);
		void handleDrag();
		void handleHover();
		void handleQuit(bool force);

		bool hasQuit();
		long double getProgramTime();
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

		Window* getDraggingWindow();
		void setDraggingWindow(Window* window, vec2 offset = vec2(0, 0));
		void focusTo(Window* window);
		Window* getCurrentWindow();
		TextEntry* getTextEntry();
		void setTextEntry(TextEntry* textentry);

		private:

		// run() returns when quit is true
		bool quit;
		// desired time between frames, in seconds
		double render_delay;

		// the renderwindow to which all ui elements are drawn
		sf::RenderWindow renderwindow;

		// the window currently being dragged
		Window* dragging_window;
		// the mouse's relative position while dragging
		vec2 drag_offset;

		// the window currently in focus
		Window* current_window;

		// the text entry currently being typed into
		TextEntry* text_entry;

		// the window that was last clicked
		Window* click_window;
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
		std::map<std::pair<sf::Keyboard::Key, std::vector<sf::Keyboard::Key>>, std::function<void()>> commands;

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