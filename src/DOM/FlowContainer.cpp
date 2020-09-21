#include <OFC/DOM/FlowContainer.hpp>

#include <cmath>

namespace ofc::ui::dom {

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

    void FlowContainer::adopt(std::unique_ptr<Element> e, LayoutStyle style, const Element* beforeSibling){
        assert(e);
        assert(!beforeSibling || beforeSibling->getParentContainer() == this);
        const auto sameElem = [&](const LayoutObject& lo){
            if (auto pe = std::get_if<ElementLayout>(&lo)) {
                return pe->element == beforeSibling;
            }
            return false;
        };
        auto it = beforeSibling ?
            std::find_if(m_layout.begin(), m_layout.end(), sameElem) :
            m_layout.end();
        m_layout.insert(it, ElementLayout{e.get(), style});
        Container::adopt(std::move(e));
    }

    vec2 FlowContainer::update(){
        // TODO: margins
        // TODO: other layout styles apart from inline

        const auto avail = size();
        auto max = vec2{};
        bool firstOfLine = true;
        float x = m_padding;
        float y = m_padding;
        float nextY = 0.0f;
        for (auto lo : m_layout){
            if (auto pws = std::get_if<WhiteSpace>(&lo)) {
                // TODO: handle whitespace
            } else if (auto pel = std::get_if<ElementLayout>(&lo)) {
                auto e = pel->element;
                assert(e);
                e->setPos({x, y});
                auto s = e->size();
                x = std::ceil(x + s.x);
                if (x >= avail.x && !firstOfLine){
                    e->setPos({0.0, nextY});
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
        }
        return {max.x + 2.0f * m_padding, max.y + 2.0f * m_padding};
    }

    void FlowContainer::onRemoveChild(const Element* e){
        m_layout.erase(std::remove_if(
            m_layout.begin(),
            m_layout.end(),
            [&](const LayoutObject& lo){
                if (auto el = std::get_if<ElementLayout>(&lo)){
                    return el->element == e;
                }
                return false;
            }
        ), m_layout.end());
    }

} // namespace ofc::ui::dom
