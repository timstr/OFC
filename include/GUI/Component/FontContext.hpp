#pragma once

#include <GUI/Component/Context.hpp>

namespace ui {

    class UseFont : public ContextProvider<const sf::Font*, UseFont> {
    public:
        UseFont(PropertyOrValue<const sf::Font*>);
    };

    template<typename Derived>
    class FontConsumer : ContextConsumer<Derived, UseFont> {
    public:
        FontConsumer(void (Derived::* onUpdate)(const sf::Font*))
            : ContextConsumer(onUpdate) {

        }

        Derived& font(PropertyOrValue<const sf::Font*> pv) {
            getObserver().assign(std::move(pv));
            return *this;
        }

    protected:
        Observer<const sf::Font*>& getFont() {
            return getObserver();
        }
    };

} // namespace ui
