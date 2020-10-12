#pragma once

#include <OFC/DOM/Container.hpp>

#include <OFC/Util/Direction.hpp>

namespace ofc::ui::dom {

    // TODO: alignment options (left/center/right/justify)

    namespace detail {

        class VerticalTag {};
        class HorizontalTag {};

        template<typename Tag>
        class ListContainer : public Container {
        public:
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
                return m_cells[i];
            }

            void insert(std::size_t index, std::unique_ptr<Element>) {
                assert(e);
                assert(index <= m_cells.size());
                m_cells.insert(m_cells.begin() + index, e.get());
                adopt(std::move(e));
            }

            template<typename T, typename... Args>
            T& insert(std::size_t index, Args&&... args) {
                static_assert(std::is_base_of_v<Element, T>);
                static_assert(std::is_constructible_v<Element, Args...>);
                auto e = std::make_unique<T>(std::forward<Args>(args)...);
                auto& er = *e;
                this->insert(index, std::move(e));
                return er;
            }

            void erase(std::size_t index) {
                assert(index < m_cells.size());
                auto e = m_cells[index];
                release(e);
                // NOTE: m_cells is modified in onRemoveChild
            }
        
            void insertBefore(const Element* sibling, std::unique_ptr<Element> theElement) {
                if (sibling) {
                    auto it = std::find(m_cells.begin(), m_cells.end(), sibling);
                    assert(it != m_cells.end());
                    m_cells.insert(it, theElement.get());
                    adopt(std::move(theElement));
                } else {
                    push_back(std::move(theElement));
                }
            }

            void insertAfter(const Element* sibling, std::unique_ptr<Element> theElement) {
                if (sibling) {
                    auto it = std::find(m_cells.begin(), m_cells.end(), sibling);
                    assert(it != m_cells.end());
                    ++it;
                    m_cells.insert(it, theElement.get());
                    adopt(std::move(theElement));
                } else {
                    push_front(std::move(theElement));
                }
            }

            void push_front(std::unique_ptr<Element> e) {
                assert(e);
                m_cells.insert(m_cells.begin(), e.get());
                adopt(std::move(e));
            }

            template<typename T, typename... Args>
            T& push_front(Args&&... args) {
                static_assert(std::is_base_of_v<Element, T>);
                static_assert(std::is_constructible_v<Element, Args...>);
                auto e = std::make_unique<T>(std::forward<Args>(args)...);
                auto& er = *e;
                this->push_front(std::move(e));
                return er;
            }

            void pop_front() {
                assert(m_cells.size() > 0);
                if (m_cells.size() == 0){
                    throw std::runtime_error("Attempted to erase from empty list");
                }
                release(m_cells.front());
                // NOTE: m_cells is modified in onRemoveChild
            }

            void push_back(std::unique_ptr<Element> e) {
                assert(e);
                m_cells.push_back(e.get());
                adopt(std::move(e));
            }

            template<typename T, typename... Args>
            T& push_back(Args&&... args) {
                static_assert(std::is_base_of_v<Element, T>);
                static_assert(std::is_constructible_v<Element, Args...>);
                auto e = std::make_unique<T>(std::forward<Args>(args)...);
                auto& er = *e;
                this->push_back(std::move(e));
                return er;
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
                , m_padding(0.0f) {

            }

        private:
            vec2 update() override {
                const auto placeItems = [&](std::optional<float> maxSize){
                    auto pos = 0.0f;
                    auto oppositeSize = 0.0f; 
                    for (std::size_t i = 0, iEnd = m_cells.size(); i != iEnd; ++i){
                        const auto ii = m_direction ? i : iEnd - 1 - i;
                        auto c = m_cells[ii];
                        assert(c);
                        assert(hasDescendent(c));

                        if (maxSize.has_value()){
                            const auto s = std::as_const(*c).size();
                            if constexpr (std::is_same_v<Tag, VerticalTag>){
                                setAvailableSize(c, vec2{*maxSize, s.y});
                            } else {
                                setAvailableSize(c, vec2{s.x, *maxSize});
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

                return totalSize;
            }

            void onRemoveChild(const Element* e) override {
                auto it = std::find(m_cells.begin(), m_cells.end(), e);
                assert(it != m_cells.end());
                m_cells.erase(it);
            }

            std::vector<Element*> m_cells;
            const bool m_direction;
            float m_padding;
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
