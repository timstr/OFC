#pragma once

#include <GUI/Container.hpp>

namespace ui {

    // TODO: are all of these still needed?
    enum class LayoutStyle {
        Inline,
        Block,
        FloatLeft,
        FloatRight,
        Free
    };

    class FlowContainer : public Container {
    public:
        FlowContainer();

        float padding() const;

        void setPadding(float);

        // write a sequence of text
        void write(const String& text, const sf::Font& font, const Color& color = {}, unsigned charsize = 15, TextStyle style = TextStyle::Regular);
        
		// write a line break, causing inline elements to continue on a new line
		void writeLineBreak();

		// write a page break, causing all elements to continue on a new line
		void writePageBreak(float height = 0.0f);

		// write a tab
		void writeTab(float width = 50.0f);

    private:
        
        vec2 update() override;

        void onRemoveChild(const Element*) override;

        float m_padding;

        struct WhiteSpace {
            enum Type {
                LineBreak,
                PageBreak,
                Tab
            };
            Type type;
            float size;
        };

        struct ElementLayout {
            Element* element;
            LayoutStyle style;
        };

        using LayoutObject = std::variant<ElementLayout, WhiteSpace>;

        std::vector<LayoutObject> m_layout;
    };

} // namespace ui
