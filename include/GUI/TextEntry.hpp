#pragma once

#include <GUI/BoxElement.hpp>
#include <GUI/Control.hpp>
#include <GUI/Text.hpp>

namespace ui {

    class TextEntry : public Control, public Text, public BoxElement {
    public:
        TextEntry(const sf::Font& font, unsigned height = 15);

        void startTyping();

        void stopTyping();

        bool isTyping() const;

    protected:

        virtual void onChange();

        virtual void onReturn();

        virtual bool validate() const;

    private:

        bool onLeftClick(int clicks) override;

        bool onKeyDown(Key) override;

        void onLoseFocus() override;

        void render(sf::RenderWindow&) override;

        void handleBackspace();
        
        void handleDelete();
        
        void handleLeft();
        
        void handleRight();
        
        void handleHome();
        
        void handleEnd();

        void handleInsert();

        void handleSelectAll();
        
        void handleCopy();
        
        void handleCut();
        
        void handlePaste();

        void handleReturn();

    private:
        virtual TextEntry* toTextEntry() override;

        void type(uint32_t unicode);

        void handleChange();

        std::pair<std::size_t, std::size_t> selection() const;

        bool shift() const;

        bool ctrl() const;

        void skipLeft();

        void skipRight();

        std::size_t m_cursorHead;
        std::size_t m_cursorTail;
        bool m_overtype;

        friend class Window;
    };


} // namespace ui