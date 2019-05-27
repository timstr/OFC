#include <GUI/FlowContainer.hpp>

namespace ui {

    float FlowContainer::padding() const {
        return m_padding;
    }

    void FlowContainer::setPadding(float v){
        m_padding = std::max(0.0f, v);
    }
    
    void FlowContainer::write(const String& text, const sf::Font& font, const Color& color, unsigned charsize, TextStyle style){
        String word;

        auto writeWord = [&, this](){
            if (word.getSize() > 0){
                this->add<Text>(word, font, color, charsize, style);
                word.clear();
            }
        };

        for (const auto& ch : text){
            if (ch == U'\n'){
                writeWord();
                writeLineBreak();
            } else if (ch == U'\t'){
                writeWord();
                writeTab();
            } else if (ch == U' '){
                writeWord();
            } else {
                word += ch;
            }
        }
        writeWord();
    }

    void FlowContainer::writeLineBreak(){
        m_layout.push_back(WhiteSpace{WhiteSpace::LineBreak});
    }

    void FlowContainer::writePageBreak(float height){
        m_layout.push_back(WhiteSpace{WhiteSpace::PageBreak, height});
    }

    void FlowContainer::writeTab(float width){
        m_layout.push_back(WhiteSpace{WhiteSpace::Tab, width});
    }

    void FlowContainer::onResize(){
        
    }

    void FlowContainer::updateContents(){
        // TODO

        const auto compute_layout = [this]() -> float {
            float inner_width = width();
            float max_width = 0.0f;

            // limits of current line
            float line_top = 0.0f;
            float line_bottom = 0.0f;
            float line_left = 0.0f;
            float line_right = inner_width;

            // horizontal position of next element
            // float next_xpos;

            // all known floating elements that are not above the current line
            std::vector<Element*> lfloat_elems;
            std::vector<Element*> rfloat_elems;

            // All inline elements that are on the current line
            std::vector<Element*> inline_elems;

            const auto newline = [&](){
                // TODO
            };
            const auto tab = [&](float size){
                // TODO
            };
            const auto page_break = [&](float size){
                // TODO
            };

            const auto add_lfloat = [&](Element* e){
                // TODO
            };
            const auto add_rfloat = [&](Element* e){
                // TODO
            };
            const auto add_inline = [&](Element* e){
                // TODO
            };
            const auto add_block = [&](Element* e){
                // TODO
            };

            for (auto& v : m_layout){
                if (std::holds_alternative<ElementLayout>(v)){
                    auto e = std::get<ElementLayout>(v);
                    switch (e.style){
                    case LayoutStyle::Block:
                        add_block(e.element);
                        break;
                    case LayoutStyle::FloatLeft:
                        add_lfloat(e.element);
                        break;
                    case LayoutStyle::FloatRight:
                        add_rfloat(e.element);
                        break;
                    case LayoutStyle::Free:
                        /* do nothing */
                        break;
                    case LayoutStyle::Inline:
                        add_inline(e.element);
                        break;
                    }
                } else {
                    auto ws = std::get<WhiteSpace>(v);
                    switch (ws.type){
                    case WhiteSpace::LineBreak:
                        newline();
                        break;
                    case WhiteSpace::PageBreak:
                        page_break(ws.size);
                        break;
                    case WhiteSpace::Tab:
                        tab(ws.size);
                        break;
                    }
                }
            }
        };
        
        // TODO
    }

} // namespace ui
