#pragma once

#include <GUI/Element.hpp>
#include <GUI/Color.hpp>
#include <GUI/Text.hpp>

#include <memory>
#include <variant>
#include <vector>

namespace ui {
    
    class Container : virtual public Element {
    public:
        void render(sf::RenderWindow&) override;

    protected:

        template<typename T, typename... Args>
        T& add(Args&&...);

        void adopt(std::unique_ptr<Element>);

        std::unique_ptr<Element> release(const Element*);

        std::vector<std::unique_ptr<Element>>& children();
        const std::vector<std::unique_ptr<Element>>& children() const;

        virtual void on_child_moved(Element*);
        virtual void on_child_resized(Element*);

    private:

        Container* toContainer() override;

        std::vector<std::unique_ptr<Element>> m_children;

        Window* m_parent_window;

        friend class Element;
        friend class Window;
    };

    // Template definitions

    template<typename T, typename... Args>
    inline T& Container::add(Args&&... args){
        static_assert(std::is_base_of_v<Element, T>, "T must derive from Element");
        T* ptr = new T(std::forward<Args>(args)...);
        T& ret = *ptr;
        adopt(std::unique_ptr<Element>(ptr));
        require_update();
        return ret;
    }

} // namespace ui