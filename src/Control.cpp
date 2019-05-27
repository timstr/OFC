#include <GUI/Control.hpp>

#include <GUI/Window.hpp>

namespace ui {
    
    bool Control::onLeftClick(int){
        return false;
    }

    bool Control::onMiddleClick(int){
        return false;
    }

    bool Control::onRightClick(int){
        return false;
    }

    void Control::onLeftRelease(){

    }

    void Control::onMiddleRelease(){

    }

    void Control::onRightRelease(){

    }

    bool Control::leftMouseDown() const {
        return false;
    }

    bool Control::middleMouseDown() const {
        return false;
    }

    bool Control::rightMouseDown() const {
        return false;
    }

    void Control::onMouseOver(Draggable*){

    }

    void Control::onMouseOut(){

    }

    bool Control::onDrop(Draggable*){
        return false;
    }

    bool Control::onKeyDown(Key){
        return false;
    }

    void Control::onKeyUp(Key){

    }

    bool Control::keyDown(Key) const {
        return false;
    }

    bool Control::onScroll(vec2){
        return false;
    }

    void Control::onGainFocus(){

    }

    void Control::onLoseFocus(){

    }

    bool Control::hasFocus() const {
        return false;
    }

    void Control::grabFocus(){
        if (auto win = getParentWindow()){
            win->focusTo(this);
        }
    }

} // namespace ui
