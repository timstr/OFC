#pragma once

#include <GUI/Control.hpp>
#include <GUI/Text.hpp>

namespace ui {

    class TextEntry final : public Control, public Text {
        TextEntry(sf::Font& font, unsigned height = 15);

        virtual void onType();

        virtual void onReturn();

        virtual bool validate() const;

    private:
        virtual TextEntry* toTextEntry() override;
    };


} // namespace ui