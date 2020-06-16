#include <GUI/DOM/HorizontalList.hpp>

namespace ui::dom {

    HorizontalList::HorizontalList()
        : m_padding(0.0f) {

    }

    std::size_t HorizontalList::length() const noexcept {
        return m_cells.size();
    }

    Element* HorizontalList::getCell(std::size_t i){
        return const_cast<Element*>(const_cast<const HorizontalList*>(this)->getCell(i));
    }

    const Element* HorizontalList::getCell(std::size_t i) const {
        assert(i < m_cells.size());
        if (i >= m_cells.size()){
            throw std::runtime_error("Invalid index");
        }
        return m_cells[i];
    }

    void HorizontalList::insert(std::size_t index, std::unique_ptr<Element> e){
        assert(e);
        assert(index <= m_cells.size());
        if (index > m_cells.size()){
            throw std::runtime_error("Invalid index");
        }
        m_cells.insert(m_cells.begin() + index, e.get());
        adopt(std::move(e));
    }

    void HorizontalList::erase(std::size_t index){
        assert(index < m_cells.size());
        if (index >= m_cells.size()){
            throw std::runtime_error("Invalid index");
        }
        auto e = m_cells[index];
        release(e);
        // NOTE: m_cells is modified in onRemoveChild
    }

    void HorizontalList::insertBefore(const Element* sibling, std::unique_ptr<Element> theElement) {
        if (sibling) {
            auto it = std::find(m_cells.begin(), m_cells.end(), sibling);
            assert(it != m_cells.end());
            m_cells.insert(it, theElement.get());
            adopt(std::move(theElement));
        } else {
            push_back(std::move(theElement));
        }
    }

    void HorizontalList::push_front(std::unique_ptr<Element> e){
        assert(e);
        m_cells.insert(m_cells.begin(), e.get());
        adopt(std::move(e));
    }

    void HorizontalList::pop_front(){
        assert(m_cells.size() > 0);
        if (m_cells.size() == 0){
            throw std::runtime_error("Attempted to erase from empty list");
        }
        release(m_cells.front());
        // NOTE: m_cells is modified in onRemoveChild
    }

    void HorizontalList::push_back(std::unique_ptr<Element> e){
        assert(e);
        m_cells.push_back(e.get());
        adopt(std::move(e));
    }

    void HorizontalList::pop_back(){
        assert(m_cells.size() > 0);
        if (m_cells.size() == 0){
            throw std::runtime_error("Attempted to erase from an empty list");
        }
    }

    void HorizontalList::clear(){
        Container::clear();
    }

    float HorizontalList::padding() const noexcept {
        return m_padding;
    }

    void HorizontalList::setPadding(float v){
        m_padding = std::max(v, 0.0f);
        requireUpdate();
    }

    vec2 HorizontalList::update(){
        auto x = 0.0f;
        auto h = 0.0f; 
        for (auto c : m_cells){
            assert(c);
            assert(hasChild(c));
            c->setPos({x + m_padding, m_padding});
            x += c->width() + 2.0f * m_padding;
            h = std::max(h, c->height());
        }
        h += 2.0f * m_padding;
        setSize({x, h});
        return {x, h};
    }

    void HorizontalList::onRemoveChild(const Element* e){
        auto it = std::find(m_cells.begin(), m_cells.end(), e);
        assert(it != m_cells.end());
        m_cells.erase(it);
    }

} // namespace ui::dom
