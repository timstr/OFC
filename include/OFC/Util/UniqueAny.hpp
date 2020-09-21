#pragma once

#include <cassert>
#include <memory>
#include <typeinfo>

namespace ofc {

    class UniqueAny {
    public:
        UniqueAny() noexcept;

        template<typename T>
        T const* getIf() const noexcept {
            auto p = m_value.get();
            if (!p) {
                assert(!m_type);
                return nullptr;
            }
            assert(m_type);
            if (typeid(T) == *m_type) {
                return static_cast<T const*>(p);
            }
            return nullptr;
        }

        bool hasValue() const noexcept;

        operator bool() const noexcept;

        void reset();

        std::type_info const* getType() const noexcept;

    private:
        std::unique_ptr<void, void(*)(void*)> m_value;
        std::type_info const* m_type;

        template<typename T, typename... Args>
        friend UniqueAny makeUniqueAny(Args&&... args);
    };

    template<typename T, typename... Args>
    UniqueAny makeUniqueAny(Args&&... args) {
        auto ret = UniqueAny{};
        auto deleter = [](void* ptr) {
            delete static_cast<T*>(ptr);
        };
        ret.m_value = std::unique_ptr<T, decltype(deleter)>{
            new T{std::forward<Args>(args)...},
            deleter
        };
        ret.m_type = &typeid(T);
        return ret;
    }

} // namespace ofc
