#include <GUI/VerticalList.hpp>

namespace ui {

    VerticalList::VerticalList()
        : m_padding(0.0f) {

    }

    std::size_t VerticalList::length() const noexcept {
        return m_cells.size();
    }

    Element* VerticalList::getCell(std::size_t i){
        return const_cast<Element*>(const_cast<const VerticalList*>(this)->getCell(i));
    }

    const Element* VerticalList::getCell(std::size_t i) const {
        assert(i < m_cells.size());
        if (i >= m_cells.size()){
            throw std::runtime_error("Invalid index");
        }
        return m_cells[i];
    }

    void VerticalList::insert(std::size_t index, std::unique_ptr<Element> e){
        assert(e);
        assert(index <= m_cells.size());
        if (index > m_cells.size()){
            throw std::runtime_error("Invalid index");
        }
        m_cells.insert(m_cells.begin() + index, e.get());
        adopt(std::move(e));
    }

    void VerticalList::erase(std::size_t index){
        assert(index < m_cells.size());
        if (index >= m_cells.size()){
            throw std::runtime_error("Invalid index");
        }
        auto e = m_cells[index];
        release(e);
        // NOTE: m_cells is modified in onRemoveChild
    }

    void VerticalList::push_front(std::unique_ptr<Element> e){
        assert(e);
        m_cells.insert(m_cells.begin(), e.get());
        adopt(std::move(e));
    }

    void VerticalList::pop_front(){
        assert(m_cells.size() > 0);
        if (m_cells.size() == 0){
            throw std::runtime_error("Attempted to erase from empty list");
        }
        release(m_cells.front());
        // NOTE: m_cells is modified in onRemoveChild
    }

    void VerticalList::push_back(std::unique_ptr<Element> e){
        assert(e);
        m_cells.push_back(e.get());
        adopt(std::move(e));
    }

    void VerticalList::pop_back(){
        assert(m_cells.size() > 0);
        if (m_cells.size() == 0){
            throw std::runtime_error("Attempted to erase from an empty list");
        }
    }

    void VerticalList::clear(){
        Container::clear();
    }

    float VerticalList::padding() const noexcept {
        return m_padding;
    }

    void VerticalList::setPadding(float v){
        m_padding = std::max(v, 0.0f);
        requireUpdate();
    }

    vec2 VerticalList::update(){
        auto y = 0.0f;
        auto w = 0.0f; 
        for (auto c : m_cells){
            assert(c);
            assert(hasDescendent(c));
            c->setPos({m_padding, y + m_padding});
            y += c->height() + 2.0f * m_padding;
            w = std::max(w, c->width());
        }
        w += 2.0f * m_padding;
        setSize({w, y});
        return {w, y};
    }

    void VerticalList::onRemoveChild(const Element* e){
        auto it = std::find(m_cells.begin(), m_cells.end(), e);
        assert(it != m_cells.end());
        m_cells.erase(it);
    }

} // namespace ui
