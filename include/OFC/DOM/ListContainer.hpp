#pragma once

#include <OFC/DOM/Container.hpp>

#include <OFC/Util/Direction.hpp>

#include <numeric>

namespace ofc::ui::dom {

    // TODO: alignment options (left/center/right/justify)

    namespace detail {

        class VerticalTag {};
        class HorizontalTag {};

        template<typename Tag>
        class ListContainer : public Container {
        public:
            enum class Style : std::uint8_t {
                Begin,
                Center,
                End,
                Left = Begin,
                Right = End,
                Top = Begin,
                Bottom = End
            };

            static_assert(std::is_same_v<Tag, VerticalTag> || std::is_same_v<Tag, HorizontalTag>);
        
            using Container::children;

            std::size_t length() const noexcept {
                return m_cells.size();
            }

            Element* getCell(std::size_t i) {
                return const_cast<Element*>(const_cast<const ListContainer*>(this)->getCell(i));
            }
            const Element* getCell(std::size_t i) const {
                assert(i < m_cells.size());
                return m_cells[i].element;
            }

            float getWeight(std::size_t i) const {
                assert(i < m_cells.size());
                return m_cells[i].weight;
            }

            void setWeight(std::size_t i, float w) {
                assert(w >= 0.0f);
                assert(i < m_cells.size());
                m_cells[i].weight = w;
            }
            
            Style style(std::size_t i) const {
                assert(i < m_cells.size());
                return m_cells[i].style;
            }

            void setStyle(std::size_t i, Style s) const {
                assert(i < m_cells.size());
                m_cells[i].style = s;
                requireUpdate();
            }

            bool expand() const noexcept {
                return m_expand;
            }

            void setExpand(bool e) noexcept {
                if (e != m_expand) {
                    m_expand = e;
                    requireUpdate();
                }
            }

            void insert(std::size_t index, std::unique_ptr<Element> e, float weight = 1.0f, Style style = Style::Center) {
                assert(e);
                assert(index <= m_cells.size());
                m_cells.insert(m_cells.begin() + index, WeightedElement{e.get(), weight, style});
                adopt(std::move(e));
            }

            void erase(std::size_t index) {
                assert(index < m_cells.size());
                auto e = m_cells[index].element;
                release(e);
                // NOTE: m_cells is modified in onRemoveChild
            }
        
            void insertBefore(const Element* sibling, std::unique_ptr<Element> theElement, float weight = 1.0f, Style style = Style::Center) {
                if (sibling) {
                    auto sameElement = [sibling](const WeightedElement& we) {
                        return we.element == sibling;
                    };
                    auto it = std::find_if(m_cells.begin(), m_cells.end(), sameElement);
                    assert(it != m_cells.end());
                    m_cells.insert(it, WeightedElement{theElement.get(), weight, style});
                    adopt(std::move(theElement));
                } else {
                    push_back(std::move(theElement), weight, style);
                }
            }

            void insertAfter(const Element* sibling, std::unique_ptr<Element> theElement, float weight = 1.0f, Style style = Style::Center) {
                if (sibling) {
                    auto sameElement = [sibling](const WeightedElement& we) {
                        return we.element == sibling;
                    };
                    auto it = std::find_if(m_cells.begin(), m_cells.end(), sameElement);
                    assert(it != m_cells.end());
                    ++it;
                    m_cells.insert(it, WeightedElement{theElement.get(), weight, style});
                    adopt(std::move(theElement));
                } else {
                    push_front(std::move(theElement), weight, style);
                }
            }

            void push_front(std::unique_ptr<Element> e, float weight = 1.0f, Style style = Style::Center) {
                assert(e);
                m_cells.insert(m_cells.begin(), WeightedElement{e.get(), weight, style});
                adopt(std::move(e));
            }

            void pop_front() {
                assert(m_cells.size() > 0);
                if (m_cells.size() == 0){
                    throw std::runtime_error("Attempted to erase from empty list");
                }
                release(m_cells.front());
                // NOTE: m_cells is modified in onRemoveChild
            }

            void push_back(std::unique_ptr<Element> e, float weight = 1.0f, Style style = Style::Center) {
                assert(e);
                m_cells.push_back(WeightedElement{e.get(), weight, style});
                adopt(std::move(e));
            }

            void pop_back() {
                assert(m_cells.size() > 0);
                if (m_cells.size() == 0){
                    throw std::runtime_error("Attempted to erase from an empty list");
                }
            }

            using Container::clear;

            using Container::release;

            float padding() const noexcept {
                return m_padding;
            }

            void setPadding(float v) {
                m_padding = std::max(0.0f, v);
                requireUpdate();
            }

        protected:
            ListContainer(bool direction)
                : m_direction(direction)
                , m_padding(0.0f)
                , m_expand(false) {

            }

        private:
            vec2 update() override {
                const auto availSize = size();
                // TODO: inspect available size (given simply by size()), then  
                // layout items normally, then, in a second pass, give additional
                // space to all elements proportional to their weight
                const auto placeItems = [&](std::optional<float> maxSizeOrtho){
                    auto pos = 0.0f;
                    auto oppositeSize = 0.0f; 
                    for (std::size_t i = 0, iEnd = m_cells.size(); i != iEnd; ++i){
                        const auto ii = m_direction ? i : iEnd - 1 - i;
                        auto c = m_cells[ii].element;
                        assert(c);
                        assert(hasDescendent(c));

                        if (maxSizeOrtho.has_value()){
                            const auto s = std::as_const(*c).size();
                            if constexpr (std::is_same_v<Tag, VerticalTag>){
                                setAvailableSize(c, vec2{*maxSizeOrtho, s.y});
                            } else {
                                setAvailableSize(c, vec2{s.x, *maxSizeOrtho});
                            }
                        } else {
                            unsetAvailableSize(c);
                        }
                        const auto sizeNeeded = getRequiredSize(c);
                        
                        if constexpr (std::is_same_v<Tag, VerticalTag>){
                            c->setPos({m_padding, pos + m_padding});
                            pos += sizeNeeded.y + 2.0f * m_padding;
                            oppositeSize = std::max(oppositeSize, sizeNeeded.x);
                        } else {
                            c->setPos({pos + m_padding, m_padding});
                            pos += sizeNeeded.x + 2.0f * m_padding;
                            oppositeSize = std::max(oppositeSize, sizeNeeded.y);
                        }
                    }
                    oppositeSize += 2.0f * m_padding;
                    if constexpr (std::is_same_v<Tag, VerticalTag>){
                        return vec2{oppositeSize, pos};
                    } else {
                        return vec2{pos, oppositeSize};
                    }
                };
                
                auto totalSize = placeItems(std::nullopt);

                totalSize = placeItems(std::is_same_v<Tag, VerticalTag> ? totalSize.x : totalSize.y);

                // Expand elements to take up available space (if any)
                if (m_expand) {
                    auto dim = std::is_same_v<Tag, VerticalTag> ? &vec2::y : &vec2::x;
                    const auto totalSpace = totalSize.*dim;
                    const auto availSpace = availSize.*dim;
                    if (totalSpace < availSpace) {
                        auto acc = [](float sum, const WeightedElement& we) -> float {
                            return sum + we.weight;
                        };
                        const auto totalWeight = reduce(begin(m_cells), end(m_cells), 0.0f, acc);

                        const auto deltaSpace = availSpace - totalSpace;
                        auto spaceAcc = 0.0f;
                        for (const auto& we : m_cells) {
                            const auto space = deltaSpace * we.weight / totalWeight;
                            const auto e = we.element;
                            assert(e);
                            auto p = e->pos();
                            const auto k = (we.style == Style::Begin) ? 0.0f : (we.style == Style::Center) ? 0.5f : 1.0f;
                            p.*dim += spaceAcc + k * space;
                            e->setPos(p);
                            spaceAcc += space;
                        }
                    }
                }

                return totalSize;
            }

            void onRemoveChild(const Element* e) override {
                auto sameElement = [e](const WeightedElement& we) {
                    return we.element == e;
                };
                auto it = std::find_if(m_cells.begin(), m_cells.end(), sameElement);
                assert(it != m_cells.end());
                m_cells.erase(it);
            }

            struct WeightedElement {
                Element* element = nullptr;
                float weight = 1.0f;
                Style style = Style::Middle;
            };

            std::vector<WeightedElement> m_cells;
            const bool m_direction;
            float m_padding;
            bool m_expand;
        };
    }



    class VerticalList : public detail::ListContainer<detail::VerticalTag> {
    public:
        VerticalList(VerticalDirection direction = TopToBottom);
    };

    class HorizontalList : public detail::ListContainer<detail::HorizontalTag> {
    public:
        HorizontalList(HorizontalDirection direction = LeftToRight);
    };

} // namespace ofc::ui::dom
