#pragma once

#include "Element.hpp"
#include "Transition.hpp"
#include "TextEntry.hpp"
#include <map>

namespace ui {

	struct Context {
		Context();

		// creates the application window and initializes rendering
		void init(unsigned width, unsigned height, std::string title, float _render_delay);

		// adds a transition to be applied when the screen redraws
		void addTransition(Transition transition);

		// applys all transitions
		void applyTransitions();

		// register a function to be called when `trigger_key` is pressed
		void addKeyboardCommand(Key trigger_key, std::function<void()> handler);

		// register a function to be called when `required_keys` are held and `trigger_key` is pressed
		void addKeyboardCommand(Key trigger_key, std::vector<Key> required_keys, std::function<void()> handler);

		// register a function to be called when the application closes.
		// If `handler` returns false, the application will not close and
		// will resume unless a forced quit is happening
		void setQuitHandler(std::function<bool()> handler);

		// calls a registered keyboard command if applicable
		// if multiple commands have the same trigger key and all
		// required keys held, the command with the most keys
		// will be called.
		// Otherwise, if no command matches, propagates onKeyDown
		// starting with current element
		void handleKeyDown(Key key);

		// invokes onKeypUp on the element which last received this key being pressed
		void handleKeyUp(Key key);

		// propagates single- or double-click starting with the element at `pos`,
		// depending on when the last element was clicked
		void handleMouseDown(sf::Mouse::Button button, vec2 pos);

		// invokes onLeftRelease or onRightRelease the element which was last clicked
		// with that button
		void handleMouseUp(sf::Mouse::Button button);

		// calls key up and mouse up handlers on any elements waiting for them
		void releaseAllButtons();

		// propagates onScroll starting with element at `pos`
		void handleScroll(vec2 pos, float delta_x, float delta_y);

		// moves the dragging element and calls onDrag
		void handleDrag();

		// calls onMouseOut on last hovered element and its ancestors until common ancester
		// is found with new element located at `pos`, it and its ancestors have onMouseOver called
		void handleHover(vec2 pos);

		// calls quit handler unless `force` is true
		void handleQuit(bool force);

		// true if the application has been quit
		bool hasQuit();

		// get the current mouse cursor position
		vec2 getMousePosition();

		// get the render window for drawing to the screen
		sf::RenderWindow& getRenderWindow();

		// get the desired time between renders
		float getRenderDelay();

		// translate the rendering context
		void translateView(vec2 offset);

		// get the rendering context's displacement from the top left corner
		vec2 getViewOffset();

		// reset the rendering context to the full window and default position
		void resetView();

		// apply changes to the rendering context
		void updateView();

		// get the on-screen coordinates of the current rendering context
		const sf::FloatRect& getClipRect();

		// set the clipping area
		void setClipRect(const sf::FloatRect& rect);

		// shrink the clipping area to that which intersects with `rect`
		void intersectClipRect(const sf::FloatRect& rect);

		// update the default screen size
		void resize(int w, int h);

		// update the cached program time
		void updateTime();

		// return the cached program time
		float getProgramTime() const;

		// get the dragging element
		Ref<Element> getDraggingElement() const;

		// set the dragging element
		void setDraggingElement(Ref<Element> element, vec2 offset = vec2(0, 0));

		// call onLoseFocus on last current element and its ancestors until common ancestor with
		// `element`, onFocus is then called on the ancesters of `element`, which becomes the new
		// current element
		void focusTo(Ref<Element> element);

		// get the current element
		Ref<Element> getCurrentElement() const;

		// get the element currently being hovered over
		Ref<Element> getHoverElement() const;

		// get the text entry currently being typed into
		Ref<TextEntry> getTextEntry() const;

		// set the current text entry
		void setTextEntry(Ref<TextEntry> textentry);

		// highlight the current element
		void highlightCurrentElement();

		// time since the current element was highlighted	
		float timeSinceHighlight() const;

	private:

		// flag for run() to stop
		bool quit;
		// desired time between frames, in seconds
		float render_delay;

		// cached current program time
		float program_time;

		// the renderwindow to which all ui elements are drawn
		sf::RenderWindow renderwindow;

		// the element currently being dragged
		Ref<Element> dragging_element;

		// the mouse's relative position while dragging
		vec2 drag_offset;

		// the element currently being hovered over
		Ref<Element> hover_element;

		// the element currently in focus
		Ref<Element> current_element;

		// time when current element was highlighted
		sf::Time highlight_timestamp;

		// the text entry currently being typed into
		Ref<TextEntry> text_entry;

		// the element that was last clicked
		Ref<Element> left_clicked_element, right_clicked_element, middle_clicked_element;
		// maximum time between clicks of a double-click, in seconds
		const float doubleclicktime;
		// time of last click
		sf::Time click_timestamp;
		// the mouse button that was last clicked
		sf::Mouse::Button click_button;


		// active transitions
		std::vector<Transition> transitions;

		sf::Clock clock;

		// keys that were pressed and which element handled them
		std::map<Key, Ref<Element>> keys_pressed;

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

	// get the global context
	Context& getContext();

} // namespace ui