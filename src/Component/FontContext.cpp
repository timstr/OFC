#include <OFC/Component/FontContext.hpp>

namespace ofc::ui {

    UseFont::UseFont(Valuelike<const sf::Font*> pvf)
        : ContextProvider(std::move(pvf)) {
    }
    
} // namespace ofc::ui
