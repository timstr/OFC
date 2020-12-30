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
    
    const FlowContainer::WhiteSpace* FlowContainer::writeLineBreak(){
        auto w = std::make_unique<WhiteSpace>(WhiteSpace::LineBreak);
        auto p = w.get();
        m_layout.push_back(std::move(w));
        return p;
    }

    const FlowContainer::WhiteSpace* FlowContainer::writePageBreak(float height){
        auto w = std::make_unique<WhiteSpace>(WhiteSpace::PageBreak, height);
        auto p = w.get();
        m_layout.push_back(std::move(w));
        return p;
    }

    const FlowContainer::WhiteSpace* FlowContainer::writeTab(float width){
        auto w = std::make_unique<WhiteSpace>(WhiteSpace::Tab, width);
        auto p = w.get();
        m_layout.push_back(std::move(w));
        return p;
    }

    void FlowContainer::remove(const Item& item) {
        if (auto epp = std::get_if<const Element*>(&item)){
            release(*epp);
        } else if (auto wspp = std::get_if<const WhiteSpace*>(&item)){
            remove(*wspp);
        }
    }

    void FlowContainer::remove(const WhiteSpace* wsp) {
        const auto match = [&](const LayoutObject& lo){
            if (auto wsupp = std::get_if<std::unique_ptr<WhiteSpace>>(&lo)) {
                return wsupp->get() == wsp;
            }
            return false;
        };

        assert(count_if(begin(m_layout), end(m_layout), match) == 1);
        auto it = find_if(begin(m_layout), end(m_layout), match);
        assert(it != end(m_layout));
        m_layout.erase(it);
        requireUpdate();
    }

    void FlowContainer::adopt(std::unique_ptr<Element> e, LayoutStyle style, const Element* beforeSibling){
        assert(e);
        // assert(!beforeSibling || beforeSibling->getParentContainer() == this);
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
        for (const auto& lo : m_layout){
            if (auto ppws = std::get_if<std::unique_ptr<WhiteSpace>>(&lo)) {
                const auto& pws = *ppws;
                if (pws->type == WhiteSpace::LineBreak) {
                    if (std::abs(y - nextY) < 1e-3f){
                        nextY = y + 15.0f;
                    }
                    y = nextY;
                } else if (pws->type == WhiteSpace::Tab) {
                    assert(pws->size > 1.0f);
                    x = pws->size * std::ceil(x / pws->size);
                } else if (pws->type == WhiteSpace::PageBreak) {
                    // TODO: handle page break (depends on floating elements)
                } else {
                    assert(false);
                }
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

    FlowContainer::WhiteSpace::WhiteSpace(Type theType, float theSize) noexcept
        : type(theType)
        , size(theSize) {

    }

} // namespace ofc::ui::dom
