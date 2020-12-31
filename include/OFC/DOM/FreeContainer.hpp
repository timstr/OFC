#pragma once

#include <OFC/DOM/Container.hpp>

#include <cstdint>

namespace ofc::ui::dom {

    // TODO per-element or per-freecontainer margins
    // TODO: offsets from edge-relative positions

    class FreeContainer : public Container {
    public:

        // How a free element is positioned relative to its parent
        enum class Style : std::uint8_t {
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

        void adopt(std::unique_ptr<Element>);
        void adopt(Style xstyle, Style ystyle, std::unique_ptr<Element>);

        using Container::release;
        
        void setElementStyle(const Element*, Style xstyle, Style ystyle);

    private:

        vec2 update() override;

        void onRemoveChild(const Element*) override;

        // mapping of all elements with fixed position style
        struct ElementStyle {
            Style x;
            Style y;
        };

        std::map<const Element*, ElementStyle> m_styles;


    };

} // namespace ofc::ui::dom
