#pragma once

#include <OFC/DOM/Element.hpp>

namespace ofc::ui::dom {
    
    class Draggable : virtual public Element {
    public:

        void startDrag();
        void stopDrag();

        virtual void onDrag();

        bool drop(vec2 localPoint = {0.0f, 0.0f});

        bool dragging() const;

    private:
        Draggable* toDraggable() override;
    };

} // namespace ofc::ui::dom
