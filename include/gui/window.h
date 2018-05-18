#pragma once

#include <SFML\Graphics.hpp>
#include <vector>
#include <functional>
#include <memory>

typedef sf::Vector2f vec2;


namespace ui {

	// TODO: fix delayed divider update by adding functions to check changes to window tree and call
	// onChange up/down tree

	/******* THOUGHTS ***********
	- allow more natural binding to data
		-> add update() method that performs custom update and updates children?
			What would that look like?


	- some common boilerplate that could use a design pattern:
	-> keeping a well-typed reference to parent window to use it in handler functions
	-> alignment is a bit messy
	-> remove and add html-style block and inline elements?
		
	New alignment:
	enum class Window::AlignmentType {
		block, inline, float
	};

	AlignmentType Window::alignment

	When windows are placed in an auto-aligning container:
	-> consecutive inline elements are appended to the same row
	-> block elements always go on a unique row

	struct Divider : Window {
		// like an HTML div, automatically aligns all child windows by their alignment type
		// and resizes itself (?), with a padding option

		Options:
			- fixed or auto-size width/height
			- align contents min/center/max, in x and y directions
			- padding: space between contained elements and to own border
	}

	*/

	using Key = sf::Keyboard::Key;

	struct Window : std::enable_shared_from_this<Window> {
		// default constructor
		Window();

		// TODO: wrap the following in getters and setters, call onChange if necessary

		// prevents the window from receiving input
		bool disabled;

		// prevents the window from rendering
		bool visible;

		// limits rendering and input to within the bounding rectangle
		bool clipping;

		enum class DisplayStyle {
			Free,
			Inline,
			Block
			// TODO: add float left/center/right and top/middle/bottom?
		};

		DisplayStyle display_style;

		// get the position (top-left corner of the window)
		vec2 getPos() const;

		// set the position (top-left corner of the window)
		void setPos(vec2 _pos);

		// get the width and height of the window
		vec2 getSize() const;

		// set the size
		void setSize(vec2 size);

		// virtual destructor for safe polymorphic destruction
		virtual ~Window();

		// clears and removes the window
		void close();

		// true if a test point (in window space) intercepts the window
		virtual bool hit(vec2 testpos) const;

		// the mouse's position relative to the window
		vec2 localMousePos() const;

		// the window's position relative to the root window
		vec2 rootPos() const;

		// called when the window is clicked on with the left mouse button
		virtual void onLeftClick(int clicks);

		// called when the left mouse button is released
		virtual void onLeftRelease();

		// called when the window is clicked on with the right mouse button
		virtual void onRightClick(int clicks);

		// called when the right mouse button is released
		virtual void onRightRelease();

		// true if the left mouse button is down and the window is in focus
		bool leftMouseDown() const;

		// true if the right mouse button is down and the window is in focus
		bool rightMouseDown() const;

		// called when the mouse is scrolled and the window is in focus
		virtual void onScroll(float delta_x, float delta_y);

		// begins the window being dragged by the mouse
		void startDrag();
		
		// called when the window is being dragged
		virtual void onDrag();

		// stops the mouse dragging the window
		void stopDrag();

		// true if the window is currently being dragged by the mouse
		bool dragging() const;

		// called when the mouse passes over the window
		virtual void onHover();

		// called when the mouse passes over the window with another window being dragged
		virtual void onHoverWithWindow(std::weak_ptr<Window> window);

		// drop the window (via the point local_pos, in local coordinates) onto the window below it
		void drop(vec2 local_pos);

		// called when a dragged window is released over the window
		// TODO: rethink the following: shall return false if the parent's method is to be invoked
		// TODO: think about how introduction of stopDrag() and deprecation of automatic drag stopping affects this
		virtual bool onDropWindow(std::weak_ptr<Window> window);

		// called when the window gains focus
		virtual void onFocus();

		// true if the window is in focus
		bool inFocus() const;

		// called when the window loses focus
		virtual void onLoseFocus();

		// brings the window into focus
		void grabFocus();

		// called when a key is pressed and the window is in focus
		virtual void onKeyDown(Key key);

		// called when a key is released and the window is in focus
		virtual void onKeyUp(Key key);

		// true if 'key' is currently being pressed and the window is in focus
		bool keyDown(Key key) const;

		// add a child window
		template<typename WindowType>
		std::weak_ptr<WindowType> add(std::shared_ptr<WindowType> child){
			static_assert(std::is_base_of<Window, WindowType>::value, "WindowType must derive from Window");
			if (auto p = child->parent.lock()){
				p->release(child);
			}
			childwindows.push_back(child);
			child->parent = weak_from_this();
			wasChanged();
			return child;
		}

		// add a child window
		template<typename WindowType, typename... ArgsT>
		std::weak_ptr<WindowType> add(ArgsT&&... args){
			static_assert(std::is_base_of<Window, WindowType>::value, "WindowType must derive from Window");
			std::shared_ptr<WindowType> child = std::make_shared<WindowType>(std::forward<ArgsT>(args)...);
			childwindows.push_back(child);
			child->parent = weak_from_this();
			wasChanged();
			return child;
		}
		
		// add a child window at a desired position
		template<typename WindowType, typename... ArgsT>
		std::weak_ptr<WindowType> add(vec2 position, ArgsT&&... args){
			static_assert(std::is_base_of<Window, WindowType>::value, "WindowType must derive from Window");
			std::shared_ptr<WindowType> child = std::make_shared<WindowType>(std::forward<ArgsT>(args)...);
			child->pos = position;
			childwindows.push_back(child);
			child->parent = weak_from_this();
			wasChanged();
			return child;
		}

		// remove and destroy a child window
		void remove(std::weak_ptr<Window> window);

		// release a child window, possibly to add to another window
		std::shared_ptr<Window> release(std::weak_ptr<Window> window);

		// bring the window in front of its siblings
		void bringToFront();

		// destroy all children
		void clear();

		// makes onChange() be called just before rendering
		void wasChanged();

		// called when a child is removed or added, when the window is resized, etc
		virtual void onChange();

		// find the window at the given local coordinates, optionally excluding a given window and all its children
		std::weak_ptr<Window> findWindowAt(vec2 _pos, std::weak_ptr<Window> exclude = {});

		// render the window
		virtual void render(sf::RenderWindow& renderwindow);

		// render the window's children, translating and clipping as needed
		void renderChildWindows(sf::RenderWindow& renderwindow);
		
		// get all children
		std::vector<std::weak_ptr<Window>> getChildWindows() const;

		// get the parent window
		std::weak_ptr<Window> getParent() const;

	private:
		vec2 pos;
		vec2 size;

		bool changed;

		std::weak_ptr<Window> parent; // TODO: make weak reference
		std::vector<std::shared_ptr<Window>> childwindows; // TODO: make vector of shared pointers

		friend struct Context;
	};

} // namespace ui