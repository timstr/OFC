#include <OFC/Observer.hpp>

#include <cassert>

namespace ofc {

    namespace detail {

        using UpdateQueue = std::vector<std::pair<const void*, std::function<void()>>>;

        struct UpdateQueuePair {
            UpdateQueue inboundQueue;
            UpdateQueue outboundQueue;
        };

        UpdateQueuePair& getUpdateQueues() noexcept {
            static UpdateQueuePair theQueues;
            return theQueues;
        }

        void enqueueValueUpdater(const void* owner, std::function<void()> f) {
            assert(owner);
            auto& q = getUpdateQueues();
            q.inboundQueue.emplace_back(owner, std::move(f));
        }

        void updateAllValues() {
            auto& qs = getUpdateQueues();
            while (true) {
                assert(qs.outboundQueue.size() == 0);
                std::swap(qs.inboundQueue, qs.outboundQueue);
                qs.inboundQueue.clear();
                bool anything = false;
                for (auto& f : qs.outboundQueue) {
                    assert(f.first);
                    if (f.second) {
                        f.second();
                        anything = true;
                    }
                }
                qs.outboundQueue.clear();
                if (!anything) {
                    return;
                }
            }
        }

        void cancelUpdates(const void* owner) {
            const auto clearTheQueue = [&](UpdateQueue& q) {
                // NOTE: because the queue might currently be being iterated elsewhere,
                // the function is set to null rather than modifying the underlying container
                for (auto& [o, f] : q) {
                    if (o == owner) {
                        f = nullptr;
                    }
                }
            };
            auto& qs = getUpdateQueues();
            clearTheQueue(qs.inboundQueue);
            clearTheQueue(qs.outboundQueue);
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



    ObserverOwner::ObserverOwner() noexcept
        : m_active(true) {

    }

    ObserverOwner::ObserverOwner(ObserverOwner&& oo) noexcept
        : m_ownObservers(std::move(oo.m_ownObservers))
        , m_active(oo.m_active) {

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

    bool ObserverOwner::isActive() const noexcept {
        return m_active;
    }

    void ObserverOwner::setActive(bool active) noexcept {
        m_active = active;
    }

} // namespace ofc
