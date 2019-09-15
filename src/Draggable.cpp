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

    Draggable* Draggable::toDraggable(){
        return this;
    }

} // namespace ui
