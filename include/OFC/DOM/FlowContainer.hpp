#pragma once

#include <OFC/DOM/Container.hpp>
#include <OFC/DOM/Text.hpp>

#include <variant>

namespace ofc::ui::dom {

    class FlowContainer : public Container {
    public:
        enum class Style : std::uint8_t {
            Inline,
            Block,
            FloatLeft,
            FloatRight,
            Free
        };

        FlowContainer();

        float padding() const;

        void setPadding(float);
        
        struct WhiteSpace {
            enum Type {
                LineBreak,
                PageBreak,
                Tab
            };
            const Type type;
            const float size;

            
            WhiteSpace(Type theType = LineBreak, float theSize = 0.0f) noexcept;
        };

        using Item = std::variant<const Element*, const WhiteSpace*>;
        
        // write a line break, causing inline elements to continue on a new line
        const WhiteSpace* writeLineBreak();

        // write a page break, causing all elements to continue on a new line
        const WhiteSpace* writePageBreak(float height = 0.0f);

        // write a tab
        const WhiteSpace* writeTab(float width = 50.0f);

        void remove(const Item&);
        void remove(const WhiteSpace*);

        void adopt(std::unique_ptr<Element>, Style style = Style::Inline, const Element* beforeSibling = nullptr);

        using Container::release;

    private:
        
        vec2 update() override;

        void onRemoveChild(const Element*) override;

        float m_padding;

        struct ElementLayout {
            Element* element;
            Style style;
        };

        using LayoutObject = std::variant<ElementLayout, std::unique_ptr<WhiteSpace>>;

        std::vector<LayoutObject> m_layout;
    };

} // namespace ofc::ui::dom
