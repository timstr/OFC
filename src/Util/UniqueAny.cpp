#include <OFC/Util/UniqueAny.hpp>

namespace ofc {

    UniqueAny::UniqueAny() noexcept
        : m_value{nullptr, nullptr}
        , m_type{nullptr} {

    }

    bool UniqueAny::hasValue() const noexcept {
        assert(static_cast<bool>(m_value) == static_cast<bool>(m_type));
        return m_value.get() != nullptr;
    }

    UniqueAny::operator bool() const noexcept {
        return hasValue();
    }

    void UniqueAny::reset() {
        m_value = nullptr;
        m_type = nullptr;
    }

    std::type_info const* UniqueAny::getType() const noexcept {
        return m_type;
    }

} // namespace ofc
