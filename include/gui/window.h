#pragma once

#include <SFML\Graphics.hpp>
#include <vector>
#include <functional>
#include <set>

typedef sf::Vector2f vec2;


namespace ui {

	// TODO: improve, take inspiration from React and other UI's
	// TODO: automatic sizing based on child windows

	/******* THOUGHTS ***********
	- use shared pointers (and [const] references where possible) instead of raw pointers
		-> this will make debugging much easier and children management much simpler
		-> BEWARE: close() function is in violation of this
	- general code cleanup
		-> make methods const whenever possible
		-> reconsider how arguments are passed
		-> move semantics
		-> general cleanup of logic and stuff
	- allow more natural binding to data
		-> add update() method that performs custom update and updates children?
			What would that look like?

	- some common boilerplate that could use a design pattern:
		-> keeping a well-typed reference to parent window to use it in handler functions
		-> alignment is a bit messy
			-> remove and add html-style block and inline elements?
	*/

	struct Window { // TODO: derive from std::enable_shared_from_this?

		// prevents the window from receiving input
		bool disabled = false;

		// prevents the window from rendering
		bool visible = true;

		// limits rendering and input to within the bounding rectangle
		bool clipping = false;

		// brings the window in front of its siblings when clicked
		bool bring_to_front = false;

		// the top-left corner of the window
		vec2 pos;

		// the width and height of the window
		vec2 size;

		// virtual destructor for safe polymorphic destruction
		virtual ~Window();

		// clears and removes the window
		void close();

		// true if a test point (in window space) intercepts the window
		virtual bool hit(vec2 testpos);

		// the mouse's position relative to the window
		vec2 localMousePos() const;

		// the window's position relative to the root window
		vec2 absPos();

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
		virtual void onScroll(double delta_x, double delta_y);

		// begins the window being dragged by the mouse
		void startDrag();
		
		// called when the window is being dragged
		virtual void onDrag();

		// stops the mouse dragging the window
		void stopDrag() const;

		// true if the window is currently being dragged by the mouse
		bool dragging() const;

		// called when the mouse passes over the window
		virtual void onHover();

		// called when the mouse passes over the window with another window being dragged
		virtual void onHoverWithWindow(Window *drag_window);

		// called when a dragged window is released over the window
		// TODO: rethink the following: shall return false if the parent's method is to be invoked
		// TODO: think about how introduction of stopDrag() and deprecation of automatic drag stopping affects this
		virtual bool onDropWindow(Window *window);

		// called when the window gains focus
		virtual void onFocus();

		// true if the window is in focus
		bool inFocus() const;

		// called when the window loses focus
		virtual void onLoseFocus();

		// brings the window into focus
		void grabFocus();

		// focuses to the next window (sorted by y, then x position), if this window is in focus
		void focusToNextWindow() const;

		// focuses to the previous window (sorted by y, then x position), if this window is in focus
		void focusToPreviousWindow() const;

		// called when a key is pressed and the window is in focus
		virtual void onKeyDown(sf::Keyboard::Key key);

		// called when a key is released and the window is in focus
		virtual void onKeyUp(sf::Keyboard::Key key);

		// true if 'key' is currently being pressed and the window is in focus
		bool keyDown(sf::Keyboard::Key key) const;


		struct Alignment {
			enum Type {
				None,
				After,
				Before,
				Center,
				InsideMin,
				InsideMax
			};

			protected:
			float margin;

			Alignment(Type _type, Window* _relative_to = nullptr, float _margin = 0.0f);

			Type type;
			Window* relative_to;

			friend struct Window;
		};

		struct XAlignment : Alignment {
			private:

			XAlignment(Type type, Window* relative_to = nullptr, float margin = 0.0f);

			friend struct Window;
		};
		struct YAlignment : Alignment {
			private:

			YAlignment(Type type, Window* relative_to = nullptr, float margin = 0.0f);

			friend struct Window;
		};

		void setXAlign(XAlignment xalignment = XAlignment(Alignment::None, nullptr));
		void setYAlign(YAlignment yalignment = YAlignment(Alignment::None, nullptr));
		void align();
		void alignChildren();
		void alignAndAutoSize(float margin);

		static XAlignment noAlignX();
		static XAlignment leftOf(Window* window, float margin = 0.0f);
		static XAlignment rightOf(Window* window, float margin = 0.0f);
		static XAlignment middleOfX(Window* window);
		static XAlignment insideLeft(Window* window, float margin = 0.0f);
		static XAlignment insideRight(Window* window, float margin = 0.0f);
		static YAlignment noAlignY();
		static YAlignment above(Window* window, float margin = 0.0f);
		static YAlignment below(Window* window, float margin = 0.0f);
		static YAlignment insideTop(Window* window, float margin = 0.0f);
		static YAlignment insideBottom(Window* window, float margin = 0.0f);
		static YAlignment middleOfY(Window* window);

		// add a child window
		void addChildWindow(Window* window);
		void addChildWindow(Window* window, vec2 pos);
		void addChildWindow(Window* window, XAlignment xalignment);
		void addChildWindow(Window* window, YAlignment yalignment);
		void addChildWindow(Window* window, XAlignment xalignment, YAlignment yalignment);
		void addChildWindow(Window* window, float xpos, YAlignment yalignment);
		void addChildWindow(Window* window, XAlignment xalignment, float ypos);

		// release a childwindow, without destroying it
		void releaseChildWindow(Window* window);

		// bring the window in front of its siblings
		void bringToFront();

		// destroy all children
		void clear();

		// find the window at the given local coordinates
		Window* findWindowAt(vec2 _pos);

		// render the window
		virtual void render(sf::RenderWindow& renderwindow);

		// render the window's children, translating and clipping as needed
		void renderChildWindows(sf::RenderWindow& renderwindow);

		// register and begin a transition
		void startTransition(double duration, std::function<void(double)> transitionFn, std::function<void()> onComplete = {});

		// get all children
		const std::vector<Window*>& getChildWindows() const;

		// get the parent window
		Window* getParent() const;

	private:
		Window* parent = nullptr; // TODO: make weak reference
		std::vector<Window*> childwindows; // TODO: make vector of shared pointers

		XAlignment xalign = XAlignment(Alignment::None, nullptr);
		YAlignment yalign = YAlignment(Alignment::None, nullptr);
		bool children_aligned = false;

		friend struct Context;
	};

} // namespace ui