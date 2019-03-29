#pragma once

#ifndef TIMSGUI_CONTAINER_H
#define TIMSGUI_CONTAINER_H

#include "Element.hpp"

namespace ui {

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

    class Container : public virtual Element {
        // TODO
    public:
        Container();
        

		// set the horizontal alignment style
		Element& setContentAlign(ContentAlign style) noexcept;

		// get the horizontal alignment style
		ContentAlign contentAlign() const noexcept;

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
		StrongRef<ElementType> add(ArgsT&&... args) noexcept;

		// adopt an existing child element
		void adopt(StrongRef<Element> child) noexcept;

		// remove and destroy a child element
		void remove(const StrongRef<Element>& element) noexcept;

		// release a child element, possibly to add to another element
		// returns nullptr if the element is not found
		StrongRef<Element> release(const StrongRef<Element>& element) noexcept;

		// returns true if `child` directly belongs to this element
		bool has(const StrongRef<Element>& child) const noexcept;

		// get all children
		const std::vector<StrongRef<Element>>& children() const noexcept;

		// destroy all children
		void clear() noexcept;

		// find the element at the given local coordinates, optionally excluding a given element and all its children
		StrongRef<Element> findElementAt(vec2 _pos, const Element* = nullptr);


    private:
		ContentAlign m_contentalign;

		using LayoutIndex = float;

        // TODO: this should probably be a virtual method defined by Element
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
		
		std::vector<std::pair<StrongRef<Element>, WhiteSpace>> sortChildrenByLayoutIndex() const noexcept;

		std::vector<StrongRef<Element>> m_children;
		std::vector<WhiteSpace> m_whitespaces;
    };

} // namespace ui

#endif // TIMSGUI_CONTAINER_H
