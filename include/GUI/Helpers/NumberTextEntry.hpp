#pragma once

#include <GUI/TextEntry.hpp>

namespace ui {

    template<typename T>
    class NumberTextEntry : public TextEntry {
    public:
        NumberTextEntry(T default_val, T min, T max, const sf::Font& font, std::function<void(T)> on_return);
        // TODO
    };

} // namespace ui

