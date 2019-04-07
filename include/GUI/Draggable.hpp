#pragma once

namespace ui {
    
    class Draggable : virtual Element {

        void startDrag();
        void stopDrag();

        // Note: onMove can replace onDrag

    private:
        Draggable* toDraggable() override;
    };

} // namespace ui