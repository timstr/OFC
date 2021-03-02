#pragma once

#include <OFC/Component/MixedComponent.hpp>

namespace ofc::ui {

    namespace mix {
        
        void ValueDraggable::setValue(UniqueAny value) {
            m_value = std::move(value);
        }

        void ValueDraggable::clearValue() {
            m_value.reset();
        }

        const UniqueAny& ValueDraggable::getValue() const noexcept {
            return m_value;
        }

    } // namespace mix

} // namespace ofc::ui
