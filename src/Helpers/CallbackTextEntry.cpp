#include <GUI/Helpers/CallbackTextEntry.hpp>


#include <cassert>

namespace ui {

    CallbackTextEntry::CallbackTextEntry(
            const sf::Font& font,
            std::function<void(const String&)> onReturn,
            std::function<bool(const String&)> validate,
            std::function<void(const String&)> onChange
        )
        : TextEntry(font)
        , m_onReturn(std::move(onReturn))
        , m_validate(std::move(validate))
        , m_onChange(std::move(onChange)) {
        
    }

    void CallbackTextEntry::onType(){
        if (m_onChange){
            m_onChange(text());
        }
    }

    void CallbackTextEntry::onReturn(){
        assert(m_onReturn);
        m_onReturn(text());
    }

    bool CallbackTextEntry::validate() const {
        if (m_validate){
            return m_validate(text());
        }
        return true;
    }

}
