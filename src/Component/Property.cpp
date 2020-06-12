#include <GUI/Component/Property.hpp>

#include <GUI/Component/Component.hpp>

#include <cassert>

namespace ui {

    namespace detail {

        std::vector<std::function<void()>>& propertyUpdateQueue() noexcept {
            static std::vector<std::function<void()>> theQueue;
            return theQueue;
        }

        void unqueuePropertyUpdater(std::function<void()> f) {
            auto& q = propertyUpdateQueue();
            q.push_back(std::move(f));
        }

        void updateAllProperties() {
            while (true) {
                const auto q = std::move(propertyUpdateQueue());
                if (q.empty()) {
                    return;
                }
                for (auto& f : q) {
                    f();
                }
            }
        }
    }


    ObserverBase::ObserverBase(ObserverOwner* owner)
        : m_owner(owner) {

        assert(m_owner);
        addSelfTo(m_owner);
    }

    ObserverBase::ObserverBase(ObserverBase&& o) noexcept
        : m_owner(std::exchange(o.m_owner, nullptr)) {

        if (m_owner){
            o.removeSelfFrom(m_owner);
            addSelfTo(m_owner);
        }
    }

    ObserverBase& ObserverBase::operator=(ObserverBase&& o) {
        if (&o == this) {
            return *this;
        }
        if (m_owner) {
            removeSelfFrom(m_owner);
        }
        if (o.m_owner) {
            o.removeSelfFrom(o.m_owner);
        }
        m_owner = std::exchange(o.m_owner, nullptr);
        if (m_owner) {
            addSelfTo(m_owner);
        }
        return *this;
    }

    ObserverOwner* ObserverBase::owner() noexcept {
        return m_owner;
    }

    const ObserverOwner* ObserverBase::owner() const noexcept {
        return m_owner;
    }

    void ObserverBase::addSelfTo(ObserverOwner* oo) {
        auto& v = oo->m_ownObservers;
        assert(std::count(v.begin(), v.end(), this) == 0);
        v.push_back(this);
    }

    void ObserverBase::removeSelfFrom(ObserverOwner* oo) {
        auto& v = oo->m_ownObservers;
        assert(std::count(v.begin(), v.end(), this) == 1);
        auto it = std::find(v.begin(), v.end(), this);
        assert(it != v.end());
        v.erase(it);
    }

    ObserverBase::~ObserverBase() {
        if (m_owner){
            removeSelfFrom(m_owner);
        }
    }

    ObserverOwner::ObserverOwner(ObserverOwner&& oo) noexcept
        : m_ownObservers(std::move(oo.m_ownObservers)) {

        for (auto& o : m_ownObservers) {
            assert(o->m_owner == &oo);
            o->m_owner = this;
        }
    }

    ObserverOwner::~ObserverOwner() noexcept {
        // NOTE: it is expected that all own observers are somehow stored as members in
        // derived classes, and that they remove themselves from this vector when destroyed.
        assert(m_ownObservers.empty());
    }

} // namespace ui
