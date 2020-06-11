#include <GUI/Component/FontContext.hpp>

namespace ui {

    UseFont::UseFont(PropertyOrValue<const sf::Font*> pvf)
        : ContextProvider(std::move(pvf)) {
    }
    
} // namespace ui
