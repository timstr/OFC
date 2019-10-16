#pragma once

#include <GUI/Element.hpp>

namespace ui {
    
    class Draggable : virtual public Element {
    public:

        void startDrag();
        void stopDrag();

        virtual void onDrag();

        void drop(vec2 local_point = {0.0f, 0.0f});

        bool dragging() const;

    private:
        Draggable* toDraggable() override;
    };

} // namespace ui