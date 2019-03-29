#pragma once

#ifndef TIMSGUI_CONTROL_H
#define TIMSGUI_CONTROL_H

#include "Element.hpp"

namespace ui {

    class Control : public virtual Element {
        // TODO

    public:
        // prevent the element from receiving input
		Control& disable() noexcept;

		// allow the element the receive input
		Control& enable() noexcept;

		// returns true if the element can receive input
		bool isEnabled() const noexcept;

		// allow child elements to be navigated using the keyboard
		Control& enableKeyboardNavigation() noexcept;

		// prevent child elements from being navigated using the keyboard
		Control& disableKeyboardNavigation() noexcept;

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
		virtual bool onHoverWith(const StrongRef<Element>& element);

		// drop the element (via the point local_pos, in local coordinates) onto the element below it
		void drop(vec2 local_pos) noexcept;

		// called when a dragged element is released over the element
		// shall return false if the parent's method is to be invoked
		// if false is returned, call will propagate to the parent
		virtual bool onDrop(const StrongRef<Element>& element);

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

        // When called during an event handler such as onLeftClick or onKeyDown,
		// transfers the corresponding response event such as onLeftRelease or onKeyUp
		// to the given element.
		// Otherwise, if not called during an event handler, does nothing.
		void transferResponseTo(const Ref<Element>& elem) const;

    private:
		bool m_disabled;
		bool m_keyboard_navigable;
    };

} // namespace ui

#endif // TIMSGUI_CONTROL_H
