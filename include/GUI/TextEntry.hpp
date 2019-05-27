#pragma once

#include <GUI/Control.hpp>
#include <GUI/Text.hpp>

namespace ui {

    class TextEntry : public Control, public Text {
    public:
        TextEntry(sf::Font& font, unsigned height = 15);

        void startTyping();

        void stopTyping();

        bool isTyping() const;

        // called immediately after the character is typed
        virtual void onType(uint32_t unicode);

        virtual void onReturn();

        virtual bool validate() const;

        bool onKeyDown(Key) override;

    private:
        virtual TextEntry* toTextEntry() override;

        void type(uint32_t unicode);

        unsigned m_cursor_begin;
        unsigned m_cursor_end;

        friend class Window;
    };


} // namespace ui