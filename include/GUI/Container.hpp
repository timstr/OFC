#pragma once

#include <GUI/Element.hpp>

#include <memory>
#include <vector>

namespace ui {
    
    class Container : virtual Element {
    public:
        Container();
        ~Container();

        float padding() const;

        void setPadding(float);

        template<typename T, typename... Args>
        T& add(Args&&...);

        void adopt(std::unique_ptr<Element>);

        std::unique_ptr<Element> release(const Element&);

        void onResize() override;

        void render(sf::RenderWindow&) override;

    private:
        Control* toControl() override;

    private:
        std::vector<std::unique_ptr<Element>> m_children;

        float m_padding;
        bool m_dirty;
    };

} // namespace ui