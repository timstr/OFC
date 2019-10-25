#include <GUI/Draggable.hpp> 

#include <GUI/Window.hpp>

namespace ui {


    void Draggable::startDrag(){
        if (auto win = getParentWindow()){
            win->startDrag(this, localMousePos());
        }
    }

    void Draggable::stopDrag(){
        if (auto win = getParentWindow()){
            if (win->currentDraggable() == this){
                win->stopDrag();
            }
        }
    }

    void Draggable::onDrag(){
    
    }

    bool Draggable::drop(vec2 localPoint){
        if (auto win = getParentWindow()){
            return win->dropDraggable(this, rootPos() + localPoint);
        }
        return false;
    }

    bool Draggable::dragging() const {
        if (auto win = getParentWindow()){
            return win->currentDraggable() == this;
        }
        return false;
    }

    Draggable* Draggable::toDraggable(){
        return this;
    }

} // namespace ui
