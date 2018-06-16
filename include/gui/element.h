#pragma once

#include <SFML\Graphics.hpp>
#include <vector>
#include <functional>
#include <memory>

typedef sf::Vector2f vec2;


namespace ui {

	/******* THOUGHTS ***********
	- some common boilerplate that could use a design pattern:
		- keeping a well-typed reference to parent element to use it in handler functions
		-> only really is a nuisance in buttons and text fields and sliders and so on,
		   all of which can be replaced with reusable widgets that use lambda callbacks
	*/

	using Key = sf::Keyboard::Key;

	struct Element : std::enable_shared_from_this<Element> {

		// An element's display style 
		enum class DisplayStyle {
			Free,
			Inline,
			Block
			// TODO: add float left/center/right and top/middle/bottom?
		};

		// default constructor
		Element(DisplayStyle _display_style);

		// virtual destructor for safe polymorphic destruction
		virtual ~Element();

		// clears and removes the element from its parent
		void close();

		// returns true if the element has been closed
		bool isClosed() const;

		// called when the element is closed; to be used for releasing resources reliably
		virtual void onClose();

		template<typename ElementType>
		std::shared_ptr<ElementType> getThisAs(){
			static_assert(std::is_base_of<Element, ElementType>::value, "ElementType must derive from ui::Element");
			return std::dynamic_pointer_cast<ElementType, Element>(shared_this);
		}

		// prevent the element from receiving input
		void disable();

		// allow the element the receive input
		void enable();

		// returns true if the element can receive input
		bool isEnabled() const;

		// set the visibility of the element
		void setVisible(bool is_visible);

		// returns true if the element is visible
		bool isVisible() const;

		// when true, limits rendering and input to within the bounding rectangle
		void setClipping(bool _clipping);

		// get the position (top-left corner of the element)
		vec2 getPos() const;

		// set the position (top-left corner of the element)
		void setPos(vec2 _pos);

		// get the width and height of the element
		vec2 getSize() const;

		// set the size. Choosing force = true will set both the min and max size
		void setSize(vec2 _size, bool force = false);
		void setMinSize(vec2 _min_size);
		void setMaxSize(vec2 _max_size);

		// set the display style
		void setDisplayStyle(DisplayStyle style);

		// get the display style
		DisplayStyle getDisplayStyle() const;

		// true if a test point (in local space) intercepts the element
		virtual bool hit(vec2 testpos) const;

		// the mouse's position relative to the element
		vec2 localMousePos() const;

		// the element's position relative to the root element
		vec2 rootPos() const;

		// called when the element is clicked on with the left mouse button
		// if false is returned, call will propagate to the parent
		// if true is returned, onLeftRelease will be invoked when the button is released
		virtual bool onLeftClick(int clicks);

		// called when the left mouse button is released
		virtual void onLeftRelease();

		// called when the element is clicked on with the right mouse button
		// if false is returned, call will propagate to the parent
		// if true is returned, onRightRelease will be invoked when the button is released
		virtual bool onRightClick(int clicks);

		// called when the right mouse button is released
		virtual void onRightRelease();

		// true if the left mouse button is down and the element is in focus
		bool leftMouseDown() const;

		// true if the right mouse button is down and the element is in focus
		bool rightMouseDown() const;

		// called when the mouse is scrolled and the element is in focus
		// if false is returned, call will propagate to the parent
		virtual bool onScroll(float delta_x, float delta_y);

		// begins the element being dragged by the mouse
		void startDrag();
		
		// called when the element is being dragged
		virtual void onDrag();

		// stops the mouse dragging the element
		void stopDrag();

		// true if the element is currently being dragged by the mouse
		bool dragging() const;

		// called when the mouse moves onto the element and its children
		virtual void onMouseOver();

		// called when the mouse moves off of the element and its children
		virtual void onMouseOut();

		// true if the mouse is currently over top of the element
		bool hovering() const;

		// called when the mouse is over the element
		// if false is returned, call will propagate to the parent
		virtual bool onHover();

		// called when the mouse is over the element with another element being dragged
		// if false is returned, call will propagate to the parent
		virtual bool onHoverWith(std::shared_ptr<Element> element);

		// drop the element (via the point local_pos, in local coordinates) onto the element below it
		void drop(vec2 local_pos);

		// called when a dragged element is released over the element
		// shall return false if the parent's method is to be invoked
		// if false is returned, call will propagate to the parent
		virtual bool onDrop(std::shared_ptr<Element> element);

		// called when the element gains focus
		virtual void onFocus();

		// true if the element is in focus
		bool inFocus() const;

		// called when the element loses focus
		virtual void onLoseFocus();

		// brings the element into focus
		void grabFocus();

		// called when a key is pressed and the element is in focus
		// if false is returned, call will propagate to the parent
		// if true is returned, onKeyUp will be invoked when the key is released
		virtual bool onKeyDown(Key key);

		// called when the key is released and the element last handled this key being pressed
		virtual void onKeyUp(Key key);

		// true if 'key' is currently being pressed and the element is in focus
		bool keyDown(Key key) const;

		// add a new child element
		template<typename ElementType, typename... ArgsT>
		std::shared_ptr<ElementType> add(ArgsT&&... args){
			static_assert(std::is_base_of<Element, ElementType>::value, "ElementType must derive from Element");
			// This may look strange, but the child creates the first shared_ptr to itself
			// (so that shared_from_this is valid in the constructor) and this is how that is dealt with.
			auto rawchild = new ElementType(std::forward<ArgsT>(args)...);
			std::shared_ptr<ElementType> child = rawchild->getThisAs<ElementType>();
			adopt(child);
			return child;
		}

		// adopt an existing child element
		void adopt(std::shared_ptr<Element> child);

		// remove and destroy a child element
		void remove(std::shared_ptr<Element> element);

		// release a child element, possibly to add to another element
		std::shared_ptr<Element> release(std::shared_ptr<Element> element);

		// bring the element in front of its siblings
		void bringToFront();

		// destroy all children
		void clear();

		// find the element at the given local coordinates, optionally excluding a given element and all its children
		std::shared_ptr<Element> findElementAt(vec2 _pos, std::shared_ptr<Element> exclude = nullptr);

		// render the element
		virtual void render(sf::RenderWindow& renderwindow);
		
		// get all children
		const std::vector<std::shared_ptr<Element>>& getChildren() const;

		// get the parent element
		std::weak_ptr<Element> getParent() const;

		// set the padding; spacing between content and border
		void setPadding(float _padding);

		// get the padding; spacing between content and border
		float getPadding() const;

		// set the margin; spacing between other self and other elements
		void setMargin(float _margin);

		// get the margin; spacing between other self and other elements
		float getMargin() const;

	private:

		std::shared_ptr<Element> shared_this;

		DisplayStyle display_style;
		bool disabled;
		bool visible;
		bool clipping;
		vec2 pos;
		vec2 size;
		vec2 min_size;
		vec2 max_size;
		vec2 old_total_size;
		int layout_index;
		float padding;
		float margin;

		void makeDirty();
		bool isDirty() const;
		void makeClean();

		bool dirty;

		// returns true if a change is needed
		bool update(float width_avail);

		// position and arrange children. Returns the actual size used
		vec2 Element::arrangeChildren(float width_avail);

		// render the element's children, translating and clipping as needed
		void renderChildren(sf::RenderWindow& renderwindow);

		int getNextLayoutIndex() const;
		void organizeLayoutIndices();

		std::weak_ptr<Element> parent;
		std::vector<std::shared_ptr<Element>> children;

		friend struct Context;
		friend void run();
		friend Element& root();
	};

	struct FreeElement : Element {
		FreeElement();
	};

	struct InlineElement : Element {
		InlineElement();
	};

	struct BlockElement : Element {
		BlockElement();
	};

} // namespace ui