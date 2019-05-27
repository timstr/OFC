#include <GUI/Window.hpp>

#include <GUI/Context.hpp>

namespace ui {

    Window::Window(unsigned width, unsigned height, const String& title) :
        m_sfwindow(),
        m_focus_elem(nullptr),
        m_drag_elem(nullptr),
        m_drag_offset({0.0f, 0.0f}),
        m_hover_elem(nullptr),
        m_text_entry(nullptr),
        m_lclick_elem(nullptr),
        m_mclick_elem(nullptr),
        m_rclick_elem(nullptr),
        m_last_click_time(),
        m_last_click_btn(),
        m_keypressed_elems(),
        m_root(this) {

        m_sfwindow.create(sf::VideoMode(width, height), title);
        m_sfwindow.setVerticalSyncEnabled(true);
    }

    Window& Window::create(unsigned width, unsigned height, const String& title){
        auto pw = std::unique_ptr<Window>(new Window(width, height, title));
        auto& wr = *pw;
        Context::get().add_window(std::move(pw));
        return wr;
    }

    void Window::redraw(){
        m_sfwindow.clear(sf::Color::White);
        sf::View v;
        const auto screensize = getSize();
        v.reset({{0.0, 0.0}, screensize});
        m_sfwindow.setView(v);
        m_root.setHorizontalFill(true);
        m_root.setVerticalFill(true);
        m_root.update(getSize());
        m_root.render(m_sfwindow);
        m_sfwindow.display();
    }

    vec2 Window::getSize() const {
        auto s = m_sfwindow.getSize();
        return {static_cast<float>(s.x), static_cast<float>(s.y)};
    }

    void Window::setSize(vec2 s){
        m_sfwindow.setSize({static_cast<unsigned>(s.x), static_cast<unsigned>(s.y)});
    }

    vec2 Window::getPosition() const {
        auto p = m_sfwindow.getSize();
        return {static_cast<float>(p.x), static_cast<float>(p.y)};
    }

    void Window::setPosition(vec2 p){
        m_sfwindow.setPosition({static_cast<int>(p.x), static_cast<int>(p.y)});
    }

    vec2 Window::getMousePosition() const {
        const auto p = sf::Mouse::getPosition(m_sfwindow);
        return {static_cast<float>(p.x), static_cast<float>(p.y)};
    }

    void Window::setTitle(const String& t){
        m_sfwindow.setTitle(t);
    }

    void Window::setIcon(const sf::Image& img){
        const auto data = img.getPixelsPtr();
        if (!data){
            return;
        }
        const auto s = img.getSize();
        m_sfwindow.setIcon(s.x, s.y, data);
    }

    sf::Image Window::screenshot() const {
        const auto s = m_sfwindow.getSize();
        sf::Texture tex;
        tex.create(s.x, s.y);
        tex.update(m_sfwindow);
        return tex.copyToImage();
    }

    void Window::close(){
        Context::get().remove_window(this);
    }

    bool Window::inFocus() const {
        return m_sfwindow.hasFocus();
    }

    void Window::requestFocus(){
        m_sfwindow.requestFocus();
    }

    FreeContainer& Window::root(){
        return m_root;
    }

    const FreeContainer& Window::root() const {
        return m_root;
    }

    void Window::processEvents(){
        sf::Event event;
        while (m_sfwindow.pollEvent(event)){
            switch (event.type){
                case sf::Event::Closed: {
                    close();
                    return;
                }
                case sf::Event::Resized: {
                    vec2 size{static_cast<float>(event.size.width), static_cast<float>(event.size.height)};
                    m_root.setSize(size, true);
                }
                case sf::Event::LostFocus: {
                    releaseAllButtons();
                }
                /* case sf::Event::GainedFocus: {

                } */
                case sf::Event::TextEntered: {
                    if (m_text_entry){
                        m_text_entry->type(event.text.unicode);
                    }
                }
                case sf::Event::KeyPressed: {
                    if (m_focus_elem){
                        m_focus_elem->onKeyDown(event.key.code);
                    }
                }
                case sf::Event::KeyReleased: {
                    if (m_focus_elem){
                        m_focus_elem->onKeyUp(event.key.code);
                    }
                }
                case sf::Event::MouseWheelScrolled: {
                    // TODO
                }
                case sf::Event::MouseButtonPressed: {
                    // TODO
                }
                case sf::Event::MouseButtonReleased: {
                    // TODO
                }
                case sf::Event::MouseMoved: {
                    // TODO
                }
            }
        }
    }

    void Window::releaseAllButtons(){
        if (m_lclick_elem){
            m_lclick_elem->onLeftRelease();
            m_lclick_elem = nullptr;
        }
        if (m_mclick_elem){
            m_mclick_elem->onMiddleRelease();
            m_mclick_elem = nullptr;
        }
        if (m_rclick_elem){
            m_rclick_elem->onRightRelease();
            m_rclick_elem = nullptr;
        }
        for (auto& [key, ctrl] : m_keypressed_elems){
            ctrl->onKeyUp(key);
        }
        m_keypressed_elems.clear();
    }

    void Window::onRemoveElement(Element* elem){
        // NOTE: this function will be called during the
        // destructor of Element. Do not call any virtual functions.
        if (!elem){
            return;
        }
        if (elem == m_focus_elem){
            m_focus_elem = m_focus_elem->getParentControl();
        }
        if (elem == m_drag_elem){
            m_drag_elem->stopDrag();
        }
        if (elem == m_hover_elem){
            m_hover_elem = m_hover_elem->getParentControl();
            if (m_hover_elem){
                m_drag_offset += elem->rootPos() - m_hover_elem->rootPos();
            }
        }
        if (elem == m_text_entry){
            m_text_entry->stopTyping();
        }
        if (elem == m_lclick_elem){
            m_lclick_elem = nullptr;
        }
        if (elem == m_mclick_elem){
            m_lclick_elem = nullptr;
        }
        if (elem == m_rclick_elem){
            m_lclick_elem = nullptr;
        }
        for (auto it = m_keypressed_elems.begin(), end = m_keypressed_elems.end(); it != end; ++it){
            const auto& [key, ctrl] = *it;
            if (elem == ctrl){
                m_keypressed_elems.erase(it);
                break;
            }
        }
    }

    void Window::focusTo(Control* control){
        // TODO
    }

    void Window::startDrag(Draggable* d, vec2 o){
        m_drag_elem = d;
        m_drag_offset = o;
    }

    void Window::stopDrag(){
        m_drag_elem = nullptr;
    }

    Draggable* Window::currentDraggable(){
        return m_drag_elem;
    }

    void Window::startTyping(TextEntry* te){
        m_text_entry = te;
    }

    void Window::stopTyping(){
        m_text_entry = nullptr;
    }

    TextEntry* Window::currentTextEntry(){
        return m_text_entry;
    }

    Window::RootContainer::RootContainer(Window* w) : m_window(w) {

    }

    Window* Window::RootContainer::getWindow() const {
        return m_window;
    }

} // namespace ui
