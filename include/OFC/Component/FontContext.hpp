#pragma once

#include <OFC/Component/Context.hpp>

namespace ofc::ui {

    class UseFont : public ContextProvider<const sf::Font*, UseFont> {
    public:
        UseFont(Value<const sf::Font*>);
    };

    template<typename Derived>
    class FontConsumer : public ContextConsumer<Derived, UseFont> {
    private:
        using ContextConsumer = ContextConsumer<Derived, UseFont>;

    public:
        FontConsumer(void (Derived::* onUpdate)(const sf::Font*))
            : ContextConsumer(onUpdate) {

        }

        Derived& font(Value<const sf::Font*> pv) {
            this->getObserver().assign(std::move(pv));
            return *this;
        }

    protected:
        Observer<const sf::Font*>& getFont() {
            return this->getObserver();
        }
    };

} // namespace ofc::ui
