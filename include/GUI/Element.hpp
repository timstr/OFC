#pragma once

#ifndef TIMSGUI_ELEMENT_H
#define TIMSGUI_ELEMENT_H

#include "GUI/RoundedRectangle.hpp"

#include <SFML/Graphics.hpp>
#include <vector>
#include <functional>
#include <memory>

typedef sf::Vector2f vec2;


namespace ui {

	using Key = sf::Keyboard::Key;

	// How an element is laid out and positioned relative to its parent and siblings
	enum class LayoutStyle {

		// positioned relative to parent according only to setPos()
		Free,

		// positioned on the same line as adjacent Inline elements
		Inline,

		// positioned on a unique line, and taking up the full width available
		Block,

		// positioned left of all inline elements
		FloatLeft,

		// positioned right of all inline elements
		FloatRight
	};

	// How a free element is positioned relative to the parent
	enum class PositionStyle {
		// the element's position is left untouched
		None,

		OutsideBegin,

		// the element is positioned left of the left edge
		OutsideLeft = OutsideBegin,

		// the element is positioned above the top edge
		OutsideTop = OutsideBegin,

		InsideBegin,

		// the element is positioned inside the left edge
		InsideLeft = InsideBegin,

		// the element is positioned inside the top edge
		InsideTop = InsideBegin,

		// the element is positioned in the middle of the parent
		Center,

		InsideEnd,

		// the element is positioned inside the right edge
		InsideRight = InsideEnd,

		// the element is positioned inside the bottom edge
		InsideBottom = InsideEnd,

		OutsideEnd,

		// the element is positioned right of the right edge
		OutsideRight = OutsideEnd,

		// the element is positioned below the bottom edge
		OutsideBottom = OutsideEnd,
	};

	// How an element's inline children are aligned horizontally
	enum class ContentAlign {
		// from the left edge
		Left,

		// from the right edge
		Right,

		// centered between the left and right edges
		Center,

		// spaced to fill the entire line
		Justify
	};

	enum class TextStyle : uint8_t {
		Regular = sf::Text::Text::Style::Regular,
		Bold = sf::Text::Text::Style::Bold,
		Italic = sf::Text::Text::Style::Italic,
		Underlined = sf::Text::Text::Style::Underlined,
		StrikeThrough = sf::Text::Text::Style::StrikeThrough,
	};

	// Strong, i.e. owning reference to an element.
	// Storing this reference will prevent the referred-to
	// element from being destroyed. Care should be taken
	// to avoid cycles which would cause memory leaks.
	template<typename ElementType>
	using Ref = std::shared_ptr<ElementType>;

	// Weak, i.e. non-owning reference to an element.
	// Can be used to create a strong reference using the 'lock()' method.
	// Storing this reference will allow the referred-to element
	// to be destroyed
	template<typename ElementType>
	using WeakRef = std::weak_ptr<ElementType>;

	// construct a new Element
	// To be used in place of calling ElementType's constructor directly,
	// with the exact same arguments
	template<typename ElementType, typename... ArgsT>
	Ref<ElementType> create(ArgsT&& ...args);

	struct Element : std::enable_shared_from_this<Element> {

		// default constructor
		// NOTE: shared_from_this and thisAs<T> will work inside derived constructors
		// but polymorphic (virtual) function calls should not be made during construction
		Element(LayoutStyle _display_style) noexcept;

		// virtual destructor for safe polymorphic destruction
		virtual ~Element();

		// clears and removes the element from its parent
		void close() noexcept;

		// returns true if the element has been closed
		bool isClosed() const noexcept;

		// called when the element is closed
		// can be used for releasing resources predictably, since destruction
		// may be delayed by other references to this element
		virtual void onClose();

		// Returns a strongly-typed Ref to this of the desired type
		// or null if the conversion fails
		template<typename ElementType>
		Ref<ElementType> thisAs() noexcept {
			static_assert(std::is_base_of<Element, ElementType>::value, "ElementType must derive from ui::Element");
			return std::dynamic_pointer_cast<ElementType, Element>(shared_from_this());
		}

		// prevent the element from receiving input
		Element& disable() noexcept;

		// allow the element the receive input
		Element& enable() noexcept;

		// returns true if the element can receive input
		bool isEnabled() const noexcept;

		// allow child elements to be navigated using the keyboard
		Element& enableKeyboardNavigation() noexcept;

		// prevent child elements from being navigated using the keyboard
		Element& disableKeyboardNavigation() noexcept;

		// returns true if child elements can be navigated using the keyboard
		bool keyboardNavigable() const noexcept;

		// navigate to the first non-disabled element before this
		bool navigateToPreviousElement();

		// navigate to the first non-disabled element after this
		bool navigateToNextElement();

		// navigate to the first non-disabled descendant
		bool navigateIn();

		// navigate to the first non-disabled ancestor
		bool navigateOut();

		// set the visibility of the element
		Element& setVisible(bool _visible) noexcept;

		// returns true if the element is isVisible
		bool isVisible() const noexcept;

		// when true, limits rendering and input to within the bounding rectangle
		Element& setClipping(bool _clipping) noexcept;

		// returns true if clipping is enabled
		bool clipping() const noexcept;

		// set the position (from paren't origin to top-left corner of the element)
		Element& setPos(vec2 _pos) noexcept;

		// set the horizontal distance from the parent's origin to left edge
		Element& setLeft(float x) noexcept;

		// set the vertical distance from the parent's origin to the top edge
		Element& setTop(float y) noexcept;

		// get the position (top-left corner of the element)
		vec2 pos() const noexcept;

		// get the horizontal distance from the parent's origin to the left edge
		float left() const noexcept;

		// get the vertical distance from the parent's origin to the top edge
		float top() const noexcept;

		// set the size. Choosing force = true will set both the min and max size
		Element& setSize(vec2 _size, bool force = false) noexcept;

		// get the width and height of the element
		vec2 size() const noexcept;

		// set the maximum size
		Element& setMaxSize(vec2 _max_size) noexcept;

		// set the minimum size
		Element& setMinSize(vec2 _min_size) noexcept;

		// get the maximum size
		vec2 maxSize() const noexcept;

		// get the minimum size
		vec2 minSize() const noexcept;

		// set the minimum width
		Element& setMinWidth(float width) noexcept;

		// set the maximum width
		Element& setMaxWidth(float width) noexcept;

		// set the current size. Choosing force = true will set both the min and max width
		Element& setWidth(float width, bool force = false) noexcept;

		// get the minimum width
		float minWidth() const noexcept;

		// get the maximum width
		float maxWidth() const noexcept;

		// get the current width
		float width() const noexcept;

		// set the minimum height
		Element& setMinHeight(float height) noexcept;

		// set the maximum height
		Element& setMaxHeight(float height) noexcept;

		// set the current height. Choosing force = true will set the both the min and max height.
		Element& setHeight(float height, bool force = false) noexcept;

		// get the maximum height
		float maxHeight() const noexcept;

		// get the minimum height
		float minHeight() const noexcept;

		// get the current height
		float height() const noexcept;

		// called when the window changes size. Useful for size-dependent updates
		// that should happen after the window resizes due to layout or other cause
		virtual void onResize();

		// set the display style
		void setLayoutStyle(LayoutStyle style) noexcept;

		// get the display style
		LayoutStyle layoutStyle() const noexcept;

		// set the horizontal alignment style
		Element& setContentAlign(ContentAlign style) noexcept;

		// get the horizontal alignment style
		ContentAlign contentAlign() const noexcept;

		// Set the horizontal position style, which only applies to Free elements
		// Determines how the element is positioned relative to the edges of the parent
		// spacing has no effect if Center is selected
		void setXPositionStyle(PositionStyle style, float spacing = 0.0f) noexcept;

		// get the horizontal position style
		PositionStyle xPositionStyle() const noexcept;

		// get the spacing for the horizontal position style
		float xPositionSpacing() const noexcept;

		// Set the vertical position style, which only applies to Free elements
		// Determines how the element is positioned relative to the edges of the parent
		// spacing has no effect if Center is selected
		void setYPositionStyle(PositionStyle style, float spacing = 0.0f) noexcept;

		// get the horizontal position style
		PositionStyle yPositionStyle() const noexcept;

		// get the spacing for the horizontal position style
		float yPositionSpacing() const noexcept;

		// set the padding; spacing between content and border
		void setPadding(float _padding) noexcept;

		// get the padding; spacing between content and border
		float padding() const noexcept;

		// set the margin; spacing between other self and other elements
		void setMargin(float _margin) noexcept;

		// get the margin; spacing between other self and other elements
		float margin() const noexcept;

		// get the background color
		sf::Color backgroundColor() const noexcept;

		// set the background color
		void setBackgroundColor(sf::Color color) noexcept;

		// get the border color
		sf::Color borderColor() const noexcept;

		// set the border color
		void setBorderColor(sf::Color color) noexcept;

		// get the border radius
		float borderRadius() const noexcept;

		// set the border radius
		void setBorderRadius(float radius) noexcept;

		// get the border thickness
		float borderThickness() const noexcept;

		// set the border thickness
		void setBorderThickness(float thickness) noexcept;

		// true if a test point (in local space, relative to the element's origin) intercepts the element
		virtual bool hit(vec2 testpos) const;

		// the mouse's position relative to the element
		vec2 localMousePos() const noexcept;

		// the element's position relative to the root element
		vec2 absPos() const noexcept;

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

		// called when the element is clicked on with the middle mouse button
		// if false is returned, call will propagate to the parent
		// if true is returned, onRightRelease will be invoked when the button is released
		virtual bool onMiddleClick(int clicks);


		// called when the middle mouse button is released
		virtual void onMiddleRelease();

		// true if the left mouse button is down
		bool leftMouseDown() const noexcept;

		// true if the right mouse button is down
		bool rightMouseDown() const noexcept;

		// true if the middle mouse button is down
		bool middleMouseDown() const noexcept;

		// called when the mouse is scrolled and the element is in focus
		// if false is returned, call will propagate to the parent
		virtual bool onScroll(float delta_x, float delta_y);

		// begins the element being dragged by the mouse
		void startDrag() noexcept;

		// called when the element is being dragged
		virtual void onDrag();

		// stops the mouse dragging the element
		void stopDrag() noexcept;

		// true if the element is currently being dragged by the mouse
		bool dragging() const noexcept;

		// called when the mouse moves onto the element and its children
		virtual void onMouseOver();

		// called when the mouse moves off of the element and its children
		virtual void onMouseOut();

		// true if the mouse is currently over top of the element
		bool hovering() const noexcept;

		// called when the mouse is over the element
		// if false is returned, call will propagate to the parent
		virtual bool onHover();

		// called when the mouse is over the element with another element being dragged
		// if false is returned, call will propagate to the parent
		virtual bool onHoverWith(const Ref<Element>& element);

		// drop the element (via the point local_pos, in local coordinates) onto the element below it
		void drop(vec2 local_pos) noexcept;

		// called when a dragged element is released over the element
		// shall return false if the parent's method is to be invoked
		// if false is returned, call will propagate to the parent
		virtual bool onDrop(const Ref<Element>& element);

		// called when the element gains focus
		virtual void onFocus();

		// true if the element is in focus
		bool inFocus() const noexcept;

		// called when the element loses focus
		virtual void onLoseFocus();

		// brings the element into focus
		void grabFocus() noexcept;

		// called when a key is pressed and the element is in focus
		// if false is returned, call will propagate to the parent
		// if true is returned, onKeyUp will be invoked when the key is released
		virtual bool onKeyDown(Key key);

		// called when the key is released and the element last handled this key being pressed
		virtual void onKeyUp(Key key);

		// true if 'key' is currently being pressed
		bool keyDown(Key key) const noexcept;

		// write a sequence of text
		void write(const std::string& text, const sf::Font& font, sf::Color color = sf::Color(0xFF), unsigned charsize = 15, TextStyle style = TextStyle::Regular) noexcept;

		// write a sequence of text
		void write(const std::wstring& text, const sf::Font& font, sf::Color color = sf::Color(0xFF), unsigned charsize = 15, TextStyle style = TextStyle::Regular) noexcept;

		// write a line break, causing inline elements to continue on a new line
		void writeLineBreak(unsigned charsize = 15u) noexcept;

		// write a page break, causing all elements to continue on a new line
		void writePageBreak(float height = 0.0f) noexcept;

		// write a tab
		void writeTab(float width = 50.0f) noexcept;

		// add a new child element
		// returns null if construction failed
		template<typename ElementType, typename... ArgsT>
		Ref<ElementType> add(ArgsT&&... args) noexcept;

		// adopt an existing child element
		void adopt(Ref<Element> child) noexcept;

		// remove and destroy a child element
		void remove(const Ref<Element>& element) noexcept;

		// release a child element, possibly to add to another element
		// returns nullptr if the element is not found
		Ref<Element> release(const Ref<Element>& element) noexcept;

		// returns true if `child` directly belongs to this element
		bool has(const Ref<Element>& child) const noexcept;

		// get all children
		const std::vector<Ref<Element>>& children() const noexcept;

		// get the parent element
		std::weak_ptr<Element> parent() const noexcept;

		// layout the element before the given sibling
		void layoutBefore(const Ref<Element>& sibling) noexcept;

		// layout the element after the given sibling
		void layoutAfter(const Ref<Element>& sibling) noexcept;

		// render the element in front of its siblings, regardless of layout
		void bringToFront() noexcept;

		// destroy all children
		void clear() noexcept;

		// find the element at the given local coordinates, optionally excluding a given element and all its children
		Ref<Element> findElementAt(vec2 _pos, const Element* = nullptr);

		// render the element
		virtual void render(sf::RenderWindow& renderwindow);

	private:

		template<typename ElementType, typename ...ArgsT>
		friend Ref<ElementType> create(ArgsT&& ...args);

		std::shared_ptr<Element> m_temp_this;

		bool m_closed;

		void* operator new(size_t size);
		void operator delete(void* ptr);
		void* operator new[](size_t) = delete;
		void operator delete[](void*) = delete;

		LayoutStyle m_layoutstyle;
		ContentAlign m_contentalign;

		bool m_disabled;
		bool m_visible;
		bool m_clipping;

		bool m_keyboard_navigable;

		vec2 m_pos;
		vec2 m_size;
		vec2 m_minsize;
		vec2 m_maxsize;
		vec2 m_oldtotalsize;

		float m_layoutindex;
		float m_padding;
		float m_margin;

		void updatePosition() noexcept;
		void updateChildPositions() noexcept;

		PositionStyle m_pstyle_x, m_pstyle_y;
		float m_spacing_x, m_spacing_y;

		RoundedRectangle m_displayrect;

		void makeDirty() noexcept;
		bool isDirty() const noexcept;
		void makeClean() noexcept;

		bool m_isdirty;

		using LayoutIndex = float;

		// returns true if a change is needed
		bool update(float width_avail);

		// position and arrange children. Returns the actual size used
		vec2 Element::arrangeChildren(float width_avail);

		// render the element's children, translating and clipping as needed
		void renderChildren(sf::RenderWindow& renderwindow);

		LayoutIndex getNextLayoutIndex() const noexcept;
		void organizeLayoutIndices() noexcept;

		// returns true if this or an ancestor is in focus
		bool ancestorInFocus() const noexcept;

		struct WhiteSpace {

			enum Type {
				None,
				LineBreak,
				Tab
			};

			WhiteSpace(Type _type, LayoutIndex _layout_index, unsigned _charsize = 15u) noexcept;

			Type type;
			LayoutIndex layout_index;
			unsigned charsize;
		};

		std::vector<std::pair<Ref<Element>, WhiteSpace>> sortChildrenByLayoutIndex() const noexcept;

		std::weak_ptr<Element> m_parent;
		std::vector<Ref<Element>> m_children;
		std::vector<WhiteSpace> m_whitespaces;

		friend struct Context;
		friend void run();
		friend Element& root();
		friend struct LayoutData;
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

	struct LeftFloatingElement : Element {
		LeftFloatingElement();
	};

	struct RightFloatingElement : Element {
		RightFloatingElement();
	};

	template<typename ElementType, typename... ArgsT>
	inline Ref<ElementType> Element::add(ArgsT&& ...args) noexcept {
		auto child = create<ElementType>(std::forward<ArgsT>(args)...);
		adopt(child);
		return child;
	}

	template<typename ElementType, typename ...ArgsT>
	inline Ref<ElementType> create(ArgsT&& ...args){
		static_assert(std::is_base_of<Element, ElementType>::value, "ElementType must derive from Element");
		// This may look strange, but the Element constructor creates the first shared_ptr to itself
		// (so that shared_from_this is valid in the constructor) and this is how that is dealt with.
		Ref<Element> child = (new ElementType(std::forward<ArgsT>(args)...))->shared_from_this();

		// remove the child's strong reference to itself, allowing it to be destroyed without
		// having to call close()
		child->m_temp_this = nullptr;

		return child->thisAs<ElementType>();
	}

} // namespace ui

#endif // TIMSGUI_ELEMENT_H