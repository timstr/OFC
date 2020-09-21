#pragma once

#include <OFC/DOM/BoxElement.hpp>
#include <OFC/DOM/Control.hpp>
#include <OFC/DOM/Text.hpp>

namespace ofc::ui::dom {

    class TextEntry : public Control, public Text, public BoxElement {
    public:
        TextEntry(const sf::Font& font, unsigned height = 15);

        void startTyping();

        void stopTyping();

        bool isTyping() const;

    protected:

        virtual void onType();

        virtual void onReturn();

        virtual bool validate() const;

    private:

        bool onLeftClick(int clicks) override;

        bool onKeyDown(Key) override;

        void onLoseFocus() override;

        void render(sf::RenderWindow&) override;

        void onChange() override;

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

        friend class ::ofc::ui::Window;
    };


} // namespace ofc::ui::dom
