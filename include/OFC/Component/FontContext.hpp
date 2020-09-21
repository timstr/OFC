#pragma once

#include <OFC/Component/Context.hpp>

namespace ofc::ui {

    class UseFont : public ContextProvider<const sf::Font*, UseFont> {
    public:
        UseFont(Valuelike<const sf::Font*>);
    };

    template<typename Derived>
    class FontConsumer : ContextConsumer<Derived, UseFont> {
    public:
        FontConsumer(void (Derived::* onUpdate)(const sf::Font*))
            : ContextConsumer(onUpdate) {

        }

        Derived& font(Valuelike<const sf::Font*> pv) {
            getObserver().assign(std::move(pv));
            return *this;
        }

    protected:
        Observer<const sf::Font*>& getFont() {
            return getObserver();
        }
    };

} // namespace ofc::ui
