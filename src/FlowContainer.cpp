#include <GUI/FlowContainer.hpp>

namespace ui {

    FlowContainer::FlowContainer()
        : m_padding(5.0f) {
    }

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

    vec2 FlowContainer::update(){
        // TODO: margins

        const auto avail = size();
        auto max = vec2{};
        bool firstOfLine = true;
        float x = m_padding;
        float y = m_padding;
        float nextY = 0.0f;
        for (auto c : children()){
            c->setPos({x, y});
            auto s = c->size();
            x = std::ceil(x + s.x);
            if (x >= avail.x && !firstOfLine){
                c->setPos({0.0, nextY});
                x = std::ceil(s.x);
                y = nextY;
                firstOfLine = true;
            } else {
                firstOfLine = false;
                nextY = std::ceil(std::max(nextY, y + s.y));
            }

            //max.x = std::max(max.x, c->left() + s.x);
            //max.y = std::max(max.y, c->top() + s.y);
            max.x = std::max(max.x, s.x);
            max.y = std::max(max.y, s.y);
        }
        return {max.x + 2.0f * m_padding, max.y + 2.0f * m_padding};

        // TODO
        // Available size is simply size()
        // Position each element in textual order
        // at the given size
        // If the available size must be exceeded,
        // simply keep track of the required width and height
        // and return that.
    }

} // namespace ui
