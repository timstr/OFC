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
            enum class Style {
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

            void insertAfter(const Element* sibling, std::unique_ptr<Element> theElement, float weight = 1.0f, Style horizontalStyle = Style::Center, Style verticalStyle = Style::Center, bool expand = false) {
                auto sameElement = [sibling](const WeightedElement& we) {
                    return we.element == sibling;
                };
                auto it = std::find_if(m_cells.begin(), m_cells.end(), sameElement);
                if (it == end(m_cells)){
                    it = begin(m_cells);
                } else {
                    ++it;
                }
                m_cells.insert(it, WeightedElement{theElement.get(), weight, horizontalStyle, verticalStyle, expand});
                adopt(std::move(theElement));
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
            ListContainer(bool direction, bool expand)
                : m_direction(direction)
                , m_padding(0.0f)
                , m_expand(expand) {

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
                        const auto& c = m_cells[ii];
                        const auto e = c.element;
                        assert(e);
                        assert(hasDescendent(e));

                        if (!c.expand) {
                            setAvailableSize(e, vec2{0.0f, 0.0f});
                        } else if (maxSizeOrtho.has_value()){
                            const auto s = std::as_const(*e).size();
                            if constexpr (std::is_same_v<Tag, VerticalTag>){
                                setAvailableSize(e, vec2{*maxSizeOrtho, s.y});
                            } else {
                                setAvailableSize(e, vec2{s.x, *maxSizeOrtho});
                            }
                        } else {
                            unsetAvailableSize(e);
                        }
                        const auto sizeNeeded = getRequiredSize(e);
                        
                        if constexpr (std::is_same_v<Tag, VerticalTag>){
                            e->setPos({m_padding, pos + m_padding});
                            pos += sizeNeeded.y + 2.0f * m_padding;
                            oppositeSize = std::max(oppositeSize, sizeNeeded.x);
                        } else {
                            e->setPos({pos + m_padding, m_padding});
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

                // Expand elements to take up available space along list direction (if any)
                if (m_expand) {
                    const auto dim = std::is_same_v<Tag, VerticalTag> ? &vec2::y : &vec2::x;
                    const auto totalSpace = totalSize.*dim;
                    const auto availSpace = availSize.*dim;
                    totalSize.*dim = availSize.*dim;
                    if (totalSpace < availSpace) {
                        auto acc = [](float sum, const WeightedElement& we) -> float {
                            return sum + we.weight;
                        };
                        const auto totalWeight = reduce(begin(m_cells), end(m_cells), 0.0f, acc);

                        const auto deltaSpace = availSpace - totalSpace;
                        auto spaceAcc = 0.0f;
                        for (const auto& c : m_cells) {
                            const auto space = deltaSpace * c.weight / totalWeight;
                            const auto e = c.element;
                            assert(e);
                            auto p = e->pos();
                            const auto style = std::is_same_v<Tag, VerticalTag> ? c.verticalStyle : c.horizontalStyle;
                            const auto k = (style == Style::Begin) ? 0.0f : (style == Style::Center) ? 0.5f : 1.0f;
                            p.*dim += spaceAcc + k * space;
                            e->setPos(p);
                            spaceAcc += space;
                        }
                    }
                }

                // Align elements against list direction
                {
                    const auto dim = std::is_same_v<Tag, VerticalTag> ? &vec2::x : &vec2::y;
                    const auto availSpace = totalSize.*dim;
                    for (const auto& c : m_cells) {
                        const auto e = c.element;
                        assert(e);
                        const auto space = e->size().*dim;
                        const auto d = availSpace - space;
                        assert(d >= 0.0f);
                        const auto style = std::is_same_v<Tag, VerticalTag> ? c.horizontalStyle : c.verticalStyle;
                        const auto k = (style == Style::Begin) ? 0.0f : (style == Style::Center) ? 0.5f : 1.0f;
                        auto p = e->pos();
                        p.*dim = k * d;
                        e->setPos(p);
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
                Style verticalStyle = Style::Center;
                Style horizontalStyle = Style::Center;
                bool expand = true;
            };

            std::vector<WeightedElement> m_cells;
            const bool m_direction;
            float m_padding;
            bool m_expand;
        };
    }



    class VerticalList : public detail::ListContainer<detail::VerticalTag> {
    public:
        VerticalList(VerticalDirection direction = TopToBottom, bool expand = false);
    };

    class HorizontalList : public detail::ListContainer<detail::HorizontalTag> {
    public:
        HorizontalList(HorizontalDirection direction = LeftToRight, bool expand = false);
    };

} // namespace ofc::ui::dom
