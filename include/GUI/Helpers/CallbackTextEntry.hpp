#pragma once

#include <GUI/TextEntry.hpp>

namespace ui {

    class CallbackTextEntry : public TextEntry {
    public:
        CallbackTextEntry(
            const String& str,
            const sf::Font& font,
            std::function<void(const String&)> on_return,
            std::function<bool(const String&)> validate = {},
            std::function<void(const String&)> on_change = {}
        );

        // TODO
    };

} // namespace ui
