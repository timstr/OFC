#pragma once

#include <GUI/Container.hpp>

namespace ui {
    
    // TODO: will this ever be shared by FlowContainer?

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


    class FreeContainer : public Container {
    public:
        template<typename T, typename... Args>
        T& add(Args&&... args);

        template<typename T, typename... Args>
        T& add(PositionStyle xstyle, PositionStyle ystyle, Args&&... args);

        void adopt(std::unique_ptr<Element>);
        void adopt(std::unique_ptr<Element>, PositionStyle xstyle, PositionStyle ystyle);

        std::unique_ptr<Element> release(const Element*);
        
        void setElementStyle(const Element*, PositionStyle xstyle, PositionStyle ystyle);

    private:

        void updateContents() override;

        // mapping of all elements with fixed position style
        struct ElementStyle {
            PositionStyle x;
            PositionStyle y;
        };

        std::map<const Element*, ElementStyle> m_styles;
    };

    // Template definitions

    template<typename T, typename... Args>
    inline T& FreeContainer::add(Args&&... args){
        T& ret = Container::add<T>(std::forward<Args>(args)...);
        m_styles.try_emplace(&ret, ElementStyle{PositionStyle::None, PositionStyle::None});
        return ret;
    }

    template<typename T, typename... Args>
    inline T& FreeContainer::add(PositionStyle xstyle, PositionStyle ystyle, Args&&... args){
        T& ret = Container::add<T>(std::forward<Args>(args)...);
        m_styles.try_emplace(&ret, ElementStyle{xstyle, ystyle});
        return ret;
    }

} // namespace ui
