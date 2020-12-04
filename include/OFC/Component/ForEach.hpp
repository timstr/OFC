#pragma once

#include <OFC/Component/Component.hpp>

namespace ofc::ui {

    template<typename T>
    class ForEach : public ForwardingComponent {
    public:
        ForEach(Valuelike<std::vector<T>> pv)
            : m_observer(this, &ForEach::updateContents, std::move(pv)) {

        }

        // NOTE: due to vector reallocation, references passed to vector
        // elements in the callback function may later expire, and it is
        // strongly recommended that you do not store or capture (e.g. in
        // lambda functions) references to the vector element. Capturing
        // the index by reference however is fine and is encouraged
        ForEach& Do(std::function<AnyComponent(CRefOrValue<T>, const Value<std::size_t>&)> f) {
            assert(f);
            m_fn = std::move(f);
            return *this;
        }

        // NOTE: see above
        ForEach& Do(std::function<AnyComponent(CRefOrValue<T>)> f) {
            assert(f);
            m_fn = [f = std::move(f)](CRefOrValue<T> v, const Value<std::size_t>& /* index */){
                return f(v);
            };
            return *this;
        }

    private:
        Observer<std::vector<T>> m_observer;
        std::function<AnyComponent(CRefOrValue<T>, const Value<std::size_t>&)> m_fn;
        std::vector<std::pair<AnyComponent, std::unique_ptr<Value<std::size_t>>>> m_components;

        void onMount(const dom::Element* beforeSibling) override final {
            assert(m_fn);
            assert(m_components.empty());
            const auto& vals = m_observer.getValuelike().getOnce();
            m_components.reserve(vals.size());
            auto i = std::size_t{0};
            for (const auto& v : vals) {
                auto pi = std::make_unique<Value<std::size_t>>(i);
                m_components.emplace_back(
                    m_fn(v, *pi),
                    std::move(pi)
                );
                ++i;
            }
            for (auto& c : m_components) {
                c.first.tryMount(this, beforeSibling);
            }
        }

        void onUnmount() override final {
            for (auto it = m_components.rbegin(), itEnd = m_components.rend(); it != itEnd; ++it) {
                it->first.tryUnmount();
            }
            m_components.clear();
        }

        std::vector<const Component*> getPossibleChildren() const noexcept override final {
            auto ret = std::vector<const Component*>();
            ret.reserve(m_components.size());
            for (const auto& c : m_components) {
                ret.push_back(c.first.get());
            }
            return ret;
        }

        void updateContents(const ListOfEdits<T>& edits) {
            assert(m_fn);
            auto it = m_components.begin();
            auto i = std::size_t{0};
            for (const auto& e : edits.getEdits()) {
                if (e.insertion()) {
                    auto pi = std::make_unique<Value<std::size_t>>(i);
                    auto nextComp = (it == m_components.end()) ? getNextMountedComponent() : it->first.get();
                    auto nextElement = nextComp ? nextComp->getFirstElement() : nullptr;
                    it = m_components.emplace(
                        it,
                        m_fn(e.value(), *pi),
                        std::move(pi)
                    );
                    it->first.tryMount(this, nextElement);
                    ++it;
                    ++i;
                } else if (e.deletion()) {
                    assert(it != m_components.end());
                    it->first.tryUnmount();
                    assert(it != m_components.end());
                    it = m_components.erase(it);
                } else {
                    assert(it != m_components.end());
                    assert(e.nothing());
                    assert(it != m_components.end());
                    it->second->set(i);
                    ++it;
                    ++i;
                }
            }
        }
    };

    template<typename T>
    ForEach(const Value<std::vector<T>>&) -> ForEach<T>;

    template<typename T>
    ForEach(std::vector<T>&&) -> ForEach<T>;

    template<typename T>
    ForEach(const std::vector<T>&) -> ForEach<T>;

} // namespace ofc::ui 
