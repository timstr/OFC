#include <OFC/DOM/Control.hpp>

#include <OFC/Window.hpp>

namespace ofc::ui::dom {
    
    bool Control::onLeftClick(int, ModifierKeys){
        return false;
    }

    bool Control::onMiddleClick(int, ModifierKeys){
        return false;
    }

    bool Control::onRightClick(int, ModifierKeys){
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

    bool Control::onKeyDown(Key, ModifierKeys){
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

    bool Control::onScroll(vec2, ModifierKeys){
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

    Control* Control::toControl() noexcept {
        return this;
    }

} // namespace ofc::ui::dom
