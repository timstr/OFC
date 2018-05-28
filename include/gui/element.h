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

		enum class DisplayStyle {
			Free,
			Inline,
			Block
			// TODO: add float left/center/right and top/middle/bottom?
		};

		// default constructor
		Element(DisplayStyle _display_style);

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

		// limits rendering and input to within the bounding rectangle
		void setClipping(bool _clipping);

		// get the position (top-left corner of the element)
		vec2 getPos() const;

		// set the position (top-left corner of the element)
		void setPos(vec2 _pos);

		// get the width and height of the element
		vec2 getSize() const;

		// set the size
		void setSize(vec2 size);
		void setMinSize(vec2 size);

		// virtual destructor for safe polymorphic destruction
		virtual ~Element();

		// clears and removes the element from its parent
		void close();

		// called when the element is closed, to be used for releasing resources reliably
		virtual void onClose();

		// true if a test point (in local space) intercepts the element
		virtual bool hit(vec2 testpos) const;

		// the mouse's position relative to the element
		vec2 localMousePos() const;

		// the element's position relative to the root element
		vec2 rootPos() const;

		// called when the element is clicked on with the left mouse button
		virtual void onLeftClick(int clicks);

		// called when the left mouse button is released
		virtual void onLeftRelease();

		// called when the element is clicked on with the right mouse button
		virtual void onRightClick(int clicks);

		// called when the right mouse button is released
		virtual void onRightRelease();

		// true if the left mouse button is down and the element is in focus
		bool leftMouseDown() const;

		// true if the right mouse button is down and the element is in focus
		bool rightMouseDown() const;

		// called when the mouse is scrolled and the element is in focus
		virtual void onScroll(float delta_x, float delta_y);

		// begins the element being dragged by the mouse
		void startDrag();
		
		// called when the element is being dragged
		virtual void onDrag();

		// stops the mouse dragging the element
		void stopDrag();

		// true if the element is currently being dragged by the mouse
		bool dragging() const;

		// called when the mouse passes over the element
		virtual void onHover();

		// called when the mouse passes over the element with another element being dragged
		virtual void onHoverWith(std::weak_ptr<Element> element);

		// drop the element (via the point local_pos, in local coordinates) onto the element below it
		void drop(vec2 local_pos);

		// called when a dragged element is released over the element
		// shall return false if the parent's method is to be invoked
		virtual bool onDrop(std::weak_ptr<Element> element);

		// called when the element gains focus
		virtual void onFocus();

		// true if the element is in focus
		bool inFocus() const;

		// called when the element loses focus
		virtual void onLoseFocus();

		// brings the element into focus
		void grabFocus();

		// called when a key is pressed and the element is in focus
		virtual void onKeyDown(Key key);

		// called when a key is released and the element is in focus
		virtual void onKeyUp(Key key);

		// true if 'key' is currently being pressed and the element is in focus
		bool keyDown(Key key) const;

		// add a child element
		template<typename ElementType>
		std::weak_ptr<ElementType> add(std::shared_ptr<ElementType> child){
			static_assert(std::is_base_of<Element, ElementType>::value, "ElementType must derive from Element");
			if (auto p = child->parent.lock()){
				p->release(child);
			}
			adopt(child);
			return child;
		}

		// add a child element
		template<typename ElementType, typename... ArgsT>
		std::weak_ptr<ElementType> add(ArgsT&&... args){
			static_assert(std::is_base_of<Element, ElementType>::value, "ElementType must derive from Element");
			std::shared_ptr<ElementType> child = std::make_shared<ElementType>(std::forward<ArgsT>(args)...);
			adopt(child);
			return child;
		}

		// remove and destroy a child element
		void remove(std::weak_ptr<Element> element);

		// release a child element, possibly to add to another element
		std::shared_ptr<Element> release(std::weak_ptr<Element> element);

		// bring the element in front of its siblings
		void bringToFront();

		// destroy all children
		void clear();

		// find the element at the given local coordinates, optionally excluding a given element and all its children
		std::weak_ptr<Element> findElementAt(vec2 _pos, std::weak_ptr<Element> exclude = {});

		// render the element
		virtual void render(sf::RenderWindow& renderwindow);
		
		// get all children
		std::vector<std::weak_ptr<Element>> getChildren() const;

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

		const DisplayStyle display_style;
		bool disabled;
		bool visible;
		bool clipping;
		vec2 pos;
		vec2 size;
		vec2 min_size;
		int layout_index;
		float padding;
		float margin;

		void adopt(std::shared_ptr<Element> child);

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