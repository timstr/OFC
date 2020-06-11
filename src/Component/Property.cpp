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
			auto& q = propertyUpdateQueue();
			for (auto& f : q) {
				f();
			}
			q.clear();
		}
	}


    ObserverBase::ObserverBase(Component* owner)
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

    Component* ObserverBase::owner() noexcept {
        return m_owner;
    }

    const Component* ObserverBase::owner() const noexcept {
        return m_owner;
    }

    void ObserverBase::addSelfTo(Component* c) {
        auto& v = c->m_observers;
        assert(std::count(v.begin(), v.end(), this) == 0);
        v.push_back(this);
    }

    void ObserverBase::removeSelfFrom(Component* c) {
        auto& v = c->m_observers;
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
    
} // namespace ui
