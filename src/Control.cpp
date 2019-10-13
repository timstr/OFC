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

    void Control::onMouseOver(){

    }

    void Control::onMouseOut(){

    }

    bool Control::onHover(Draggable*){
        return false;
    }

    bool Control::onDrop(Draggable*){
        return false;
    }

    bool Control::onKeyDown(Key){
        return false;
    }

    void Control::onKeyUp(Key){

    }

    bool Control::keyDown(Key key) const {
        if (!hasFocus()){
            return false;
        }
        return sf::Keyboard::isKeyPressed(key);
    }

    bool Control::onScroll(vec2){
        return false;
    }

    void Control::onGainFocus(){

    }

    void Control::onLoseFocus(){

    }

    bool Control::hasFocus() const {
        if (const auto win = getParentWindow()){
            return win->currentControl() == this;
        }
        return false;
    }

    void Control::grabFocus(){
        if (auto win = getParentWindow()){
            win->focusTo(this);
        }
    }

    void Control::transferEventResposeTo(Control* other){
        if (auto win = getParentWindow()){
            win->transferResponseTo(other);
        }
    }

    Control* Control::toControl() noexcept {
        return this;
    }

} // namespace ui
