#include <GUI/DOM/Helpers/CallbackButton.hpp>

namespace ui::dom {

    CallbackButton::CallbackButton(const String& label, const sf::Font& font, std::function<void()> onClick)
        : m_callback(std::move(onClick))
        , m_label(add<dom::Text>(label, font, 0x000000FF))
        , m_normalColor(0xDDDDDDFF)
        , m_hoverColor(0xFFFFFFFF)
        , m_activeColor(0x999999FF)
        , m_state(State::Normal) {
    
        setBackgroundColor(m_normalColor);
        setBorderRadius(5.0f);
        setBorderColor(0x888888FF);
        setBorderThickness(2.0f);
    }

    void CallbackButton::setNormalColor(Color c){
        m_normalColor = c;
        setBackgroundColor(c);
    }

    Color CallbackButton::getNormalColor() const {
        return m_normalColor;
    }

    void CallbackButton::setHoverColor(Color c){
        m_hoverColor = c;
    }

    Color CallbackButton::getHoverColor() const {
        return m_hoverColor;
    }

    void CallbackButton::setActiveColor(Color c){
        m_activeColor = c;
    }

    Color CallbackButton::getActiveColor() const {
        return m_activeColor;
    }

    void CallbackButton::setCallback(std::function<void()> callback){
        m_callback = std::move(callback);
    }

    Text& CallbackButton::getCaption() noexcept {
        return m_label;
    }

    const Text& CallbackButton::getCaption() const noexcept {
        return m_label;
    }

    bool CallbackButton::onLeftClick(int){
        m_state = State::Active;
        clearTransitions();
        setBackgroundColor(m_activeColor);
        if (m_callback){
            m_callback();
        }
        return true;
    }

    void CallbackButton::onLeftRelease(){
        if (hit(pos() + localMousePos())){
            m_state = State::Hover;
            fadeColor(backgroundColor(), m_hoverColor);
        } else {
            m_state = State::Normal;
            fadeColor(backgroundColor(), m_normalColor);
        }
    }

    bool CallbackButton::onKeyDown(ui::Key key){
        if (key == ui::Key::Return || key == ui::Key::Space){
            fadeColor(
                m_activeColor,
                m_state == State::Hover ? m_hoverColor : m_normalColor
            );
            if (m_callback){
                m_callback();
            }
            return true;
        }
        return false;
    }

    void CallbackButton::onMouseOver(){
        if (m_state != State::Active){
            m_state = State::Hover;
            fadeColor(backgroundColor(), m_hoverColor);
        }
    }

    void CallbackButton::onMouseOut(){
        if (m_state != State::Active){
            m_state = State::Normal;
            fadeColor(backgroundColor(), m_normalColor);
        }
    }

    void CallbackButton::fadeColor(Color from, Color to){
        startTransition(
            0.1,
            [from,to,this](double t){
                setBackgroundColor(interpolate(from, to, static_cast<float>(t)));
            }
        );
    }

    void CallbackButton::onRemove(){
        m_state = State::Normal;
        setBackgroundColor(m_normalColor);
    }

} // namespace ui::dom
