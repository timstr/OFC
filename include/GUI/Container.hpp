#pragma once

#include <GUI/Element.hpp>
#include <GUI/Color.hpp>
#include <GUI/Text.hpp>

#include <memory>
#include <variant>
#include <vector>

namespace ui {
    
    /*
    TODO: make container more basic, concerned with only
    adding and removing child elements, and a base class to:

    - FlowContainer (?) : A container that lays its elements
      out in an HTML flow style

    - GridContainer    : A container that divides its
      elements among a grid, (like an HTML table) whose
      sizes can be fixed or made relative

    - Others?

    To work with both of these, a common interface for computing
    layout and available/required space should be defined at the
    Container base class level, or even at the Element level.

    Design Notes:

    Element:
        - Concerned with position and size
        - Will still hold a pointer to a Container
          as before, but this Container can now be one of many
          specializations
        - (?) Concerned with updating own size and contents,
          given available space

    Container:
        - Concerned with creating, adopting, and releasing
          child elements
        - Is concrete and can be used; elements simply stay
          where they are put

    FlowContainer:
        - Concerned with computing flow layout
        - Associates each child element with a layout style
          (inline, block, float, etc)
          TODO: does it make sense to simply resize block elements
          here? Should block elements be a different type altogether?
        - This will lend itself nicely to the existing way of
          dealing with whitespace between elements

    GridContainer:
        - Has a 2D grid of rectangles inside which elements can be
          placed
        - Number of rows/columns can be customized
        - Sizing of rows/columns can be customized. Each can be given
          a minimum/maximum size as well as a percentage/weight of
          the total available space
        - TODO: how to make an element inside a grid cell take up
          the entire area? This use case should be well-supported.
          It should probably be incorporated into the available/
          required size interface

    Horizontal/Vertical List:
        - Like one row/column of a GridContainer

    */

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

    private:

        Container* toContainer() override;

        std::vector<std::unique_ptr<Element>> m_children;

        friend class Element;
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