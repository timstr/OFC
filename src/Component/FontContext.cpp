#include <OFC/Component/FontContext.hpp>

namespace ofc::ui {

    UseFont::UseFont(Value<const sf::Font*> v)
        : ContextProvider(std::move(v)) {
    }
    
} // namespace ofc::ui
