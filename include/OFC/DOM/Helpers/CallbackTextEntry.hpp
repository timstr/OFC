#pragma once

#include <OFC/DOM/TextEntry.hpp>

#include <functional>

namespace ofc::ui::dom {

    class CallbackTextEntry : public TextEntry {
    public:
        CallbackTextEntry(
            const sf::Font& font,
            std::function<void(const String&)> onReturn,
            std::function<bool(const String&)> validate = {},
            std::function<void(const String&)> onChange = {}
        );

    private:
        void onReturn() override;

        bool validate() const override;

        void onType() override;

        std::function<void(const String&)> m_onReturn;
        std::function<bool(const String&)> m_validate;
        std::function<void(const String&)> m_onChange;
    };

} // namespace ofc::ui::dom
