#include <OFC/Component/TextField.hpp>

namespace ofc::ui {

    namespace detail {
    
        class CallbackTextEntry : public dom::TextEntry {
        public:
            CallbackTextEntry(const sf::Font& font, TextField& textFieldComponent)
                : TextEntry(font)
                , m_textFieldComponent(textFieldComponent) {
            
            }

        private:
            TextField& m_textFieldComponent;

            void onType() override {
                auto& f = m_textFieldComponent.m_onChange;
                if (f) {
                    f(text());
                }
            }

            void onReturn() override {
                auto& f = m_textFieldComponent.m_onSubmit;
                if (f) {
                    f(text());
                }
            }

            bool validate() const override {
                auto& f = m_textFieldComponent.m_validate;
                if (f) {
                    return f(text());
                }
                return true;
            }
        };

    } // namespace detail

    TextField::TextField(Valuelike<String> s)
        : FontConsumer(&TextField::updateFont)
        , m_stringObserver(this, &TextField::updateString, std::move(s)) {

    }

    TextField& TextField::onChange(std::function<void(const String&)> f) {
        m_onChange = std::move(f);
        return *this;
    }

    TextField& TextField::onSubmit(std::function<void(const String&)> f) {
        m_onSubmit = std::move(f);
        return *this;
    }

    TextField& TextField::validate(std::function<bool(const String&)> f) {
        m_validate = std::move(f);
        return *this;
    }

    std::unique_ptr<dom::TextEntry> TextField::createElement() {
        auto font = getFont().getValuelike().getOnce();
        assert(font);
        auto cbte = std::make_unique<detail::CallbackTextEntry>(*font, *this);
        cbte->setText(m_stringObserver.getValuelike().getOnce());
        return cbte;
    }

    void TextField::updateString(const String& s) {
        element()->setText(s);
    }

    void TextField::updateFont(const sf::Font* f) {
        element()->setFont(*f);
    }

} // namespace ofc::ui
