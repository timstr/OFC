#include <GUI/DOM/Control.hpp>

#include <GUI/Window.hpp>

namespace ui::dom {
    
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

    void Control::onMouseEnter(Draggable*){

    }

    void Control::onMouseLeave(Draggable*){

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
        const auto win = getParentWindow();
        if (!win){
            return false;
        }
        auto focus = win->currentControl();
        while (focus){
            if (focus == this){
                return sf::Keyboard::isKeyPressed(key);
            }
            focus = focus->getParentControl();
        }
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

} // namespace ui::dom
