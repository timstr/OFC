#include <GUI/Draggable.hpp> 

#include <GUI/Window.hpp>

namespace ui {


    void Draggable::startDrag(){
        if (auto win = getParentWindow()){
            auto mp = localMousePos();
            assert(mp);
            win->startDrag(this, *mp);
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
