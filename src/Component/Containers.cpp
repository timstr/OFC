#include <OFC/Component/Containers.hpp>

namespace ofc::ui {

    void Center::onInsertChildElement(std::unique_ptr<dom::Element> element, const Scope& scope) {
        auto s = scope;
        s.add<detail::align_CenterVertically>();
        s.add<detail::align_CenterHorizontally>();
        SimpleForwardingComponent::onInsertChildElement(std::move(element), s);
    }

    Row::Row(AnyComponent c)
        : SimpleForwardingComponent(std::move(c)) {

    }

    void Row::onInsertChildElement(std::unique_ptr<dom::Element> element, const Scope& scope) {
        auto s = scope;
        s.add<detail::grid_row>(this);
        SimpleForwardingComponent::onInsertChildElement(std::move(element), s);
    }

    Column::Column(AnyComponent c)
        : SimpleForwardingComponent(std::move(c)) {
    
    }

    void Column::onInsertChildElement(std::unique_ptr<dom::Element> element, const Scope& scope) {
        auto s = scope;
        s.add<detail::grid_column>(this);
        SimpleForwardingComponent::onInsertChildElement(std::move(element), s);
    }

    namespace detail {

        grid_row::grid_row(const Row* theRow) noexcept
            : row(theRow) {

        }

        grid_column::grid_column(const Column* theColumn) noexcept
            : column(theColumn) {
        }

    } // namespace detail

} // namespace ofc::ui
