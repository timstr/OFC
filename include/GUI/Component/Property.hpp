#pragma once

#include <cassert>
#include <functional>
#include <optional>
#include <vector>

namespace ui {

    namespace detail {

        void unqueuePropertyUpdater(std::function<void()>);

        void updateAllProperties();

    } // namespace detail


    class Component;

    // TODO: remove this
    class Pinned {
    public:
        Pinned() noexcept = default;
        ~Pinned() noexcept = default;

        Pinned(const Pinned&) = delete;
        Pinned(Pinned&&) = delete;
        Pinned& operator=(Pinned&&) = delete;
        Pinned& operator=(const Pinned&) = delete;
    };

    template<typename T>
    class ListOfEdits {
    public:
        ListOfEdits(const std::vector<T>& vecOld, const std::vector<T>& vecNew) {
            // Compute longest common subsequence using dynamic-programming table
            auto oldBegin = std::size_t{0};
            auto newBegin = std::size_t{0};
            auto oldEnd = vecOld.size();
            auto newEnd = vecNew.size();
            while (oldBegin < oldEnd && newBegin < newEnd && vecOld[oldBegin] == vecNew[newBegin]) {
                ++oldBegin;
                ++newBegin;
            }
            while (oldEnd > oldBegin && newEnd > newBegin && vecOld[oldEnd - 1] == vecNew[newEnd - 1]) {
                --oldEnd;
                --newEnd;
            }
            assert(oldBegin <= oldEnd);
            assert(newBegin <= newEnd);
            const auto m = oldEnd - oldBegin;
            const auto n = newEnd - newBegin;
            auto table = std::vector<std::size_t>((n + 1) * (m + 1), 0);
            auto getTable = [&](std::size_t i, std::size_t j) -> decltype(auto) {
                assert(i <= m);
                assert(j <= n);
                return table[(j * m) + i];
            };
            {
                for (std::size_t i = 1; i <= m; ++i) {
                    const auto& vOld = vecOld[oldBegin + i - 1];
                    for (std::size_t j = 1; j <= n; ++j) {
                        const auto& vNew = vecNew[newBegin + j - 1];
                        getTable(i, j) = (vOld == vNew) ?
                            getTable(i - 1, j - 1) + 1:
                            std::max(getTable(i - 1, j), getTable(i, j - 1));
                    }
                }
            }
            // Traverse table in reverse to recover list of edits
            std::vector<Edit> edits;
            {
                for (std::size_t i = m, j = n;;){
                    if (i > 0 && j > 0 && (vecOld[oldBegin + i - 1] == vecNew[newBegin + j - 1])){
                        edits.push_back(Edit(EditType::Nothing));
                        --i;
                        --j;
                    } else if (j > 0 && (i == 0 || getTable(i, j - 1) >= getTable(i - 1, j))) {
                        edits.push_back(Edit(EditType::Insertion, vecNew[newBegin + j - 1]));
                        --j;
                    } else if (i > 0 && (j ==0 || getTable(i, j - 1) < getTable(i - 1, j))) {
                        edits.push_back(Edit(EditType::Deletion));
                        --i;
                    } else {
                        break;
                    }   
                }
            }
            for (std::size_t i = 0; i < newBegin; ++i) {
                m_edits.push_back(Edit(EditType::Nothing));
            }
            for (auto it = edits.rbegin(), itEnd = edits.rend(); it != itEnd; ++it) {
                m_edits.push_back(std::move(*it));
            }
            for (std::size_t i = newEnd; i < vecNew.size(); ++i) {
                m_edits.push_back(Edit(EditType::Nothing));
            }
        }

        class Edit;

        const std::vector<Edit>& getEdits() const noexcept {
            return m_edits;
        }

        enum EditType : std::uint8_t {
            Deletion,
            Insertion,
            Nothing
        };

        class Edit {
        public:
            const EditType type() const noexcept {
                return m_type;
            }

            bool deletion() const noexcept {
                return m_type == EditType::Deletion;
            }

            bool insertion() const noexcept {
                return m_type == EditType::Insertion;
            }

            bool nothing() const noexcept {
                return m_type == EditType::Nothing;
            }

            const T& value() const noexcept {
                assert(m_type == EditType::Insertion);
                assert(m_value.has_value());
                return *m_value;
            }

        private:
            Edit(EditType type, std::optional<T> value = std::nullopt)
                : m_type(type)
                , m_value(std::move(value)) {

            }

            const EditType m_type;
            const std::optional<T> m_value;

            friend ListOfEdits<T>;
        };

    private:
        std::vector<Edit> m_edits;
    };

    template<typename T>
    using CRefOrValue = std::conditional_t<std::is_scalar_v<T>, T, const T&>;

    template<typename T>
    struct Difference {
        // For scalar types, the difference argument type is simply that type with all top-level
        // CV-qualifications removed. This saves users from having to type the arguments of update
        // functions for simple pointers as `const SomeType* const&` when `const SomeType*` suffices
        // For other, more complicated types, the difference argument type is a reference to const T
        using ArgType = CRefOrValue<T>;

        static std::decay_t<T> compute(const T& /* vOld */, const T& vNew) noexcept {
            return vNew;
        }
    };

    template<typename T>
    struct Difference<std::vector<T>> {
        using ArgType = const ListOfEdits<T>&;

        static ListOfEdits<T> compute(const std::vector<T>& vOld, const std::vector<T>& vNew) noexcept {
            return ListOfEdits<T>{vOld, vNew};
        }
    };

    template<typename T>
    using DiffArgType = typename Difference<T>::ArgType;

    template<typename T>
    class Property;

    template<typename T>
    class Observer;

    template<typename T>
    class Property : private Pinned {
    public:
        Property(const T& t) : m_value(t) {

        }
        Property(T&& t) : m_value(std::move(t)) {

        }

        ~Property() {
            for (auto& o : m_observers) {
                o->reset();
            }
        }

        const T& getOnce() const noexcept {
            return m_value;
        }

        T& getOnceMutable() noexcept {
            if (!m_previousValue.has_value()) {
                m_previousValue.emplace(static_cast<const T&>(m_value));
                registerForUpdate();
            }
            return m_value;
        }

        void set(const T& t) {
            if (!m_previousValue.has_value()) {
                m_previousValue.emplace(std::move(m_value));
                registerForUpdate();
            }
            m_value = t;
        }
        void set(T&& t) {
            if (!m_previousValue.has_value()) {
                m_previousValue.emplace(std::move(m_value));
                registerForUpdate();
            }
            m_value = std::move(t);
        }

        // TODO: operator= for const T& / T&&?

    private:
        T m_value;
        std::optional<T> m_previousValue;
        std::vector<Observer<T>*> m_observers;

        void purgeUpdates() {
            if (!m_previousValue.has_value()) {
                return;
            }
            if (m_previousValue == m_value) {
                m_previousValue.reset();
                return;
            }
            const auto diff = Difference<T>::compute(
                static_cast<const T&>(*m_previousValue),
                static_cast<const T&>(m_value)
            );
            m_previousValue.reset();
            for (auto& o : m_observers) {
                o->update(static_cast<const DiffArgType<T>&>(diff));
            }
        }

        void registerForUpdate() {
            detail::unqueuePropertyUpdater([this]() {
                purgeUpdates();
            });
        }

        friend Observer<T>;
    };

    template<typename T>
    class PropertyOrValue {
    public:
        // PropertyOrValue is implictly constructible from an l-value reference
        // to a matching property, in which case it will point to that property
        // and contain no fixed value.
        PropertyOrValue(Property<T>& target) noexcept
            : m_target(&target)
            , m_fixedValue(std::nullopt) {

        }

        // PropertyOrValue is implicitly constructible from anything that can be
        // used to construct a value of type T, in which case it will not point
        // to any property and will instead contain a fixed value.
        template<
            typename... Args,
            std::enable_if_t<
            !std::is_same_v<Property<T>&, Args...> &&
            std::is_constructible_v<T, Args...>
            >* = nullptr
        >
            PropertyOrValue(Args&&... args)
            : m_target(nullptr)
            , m_fixedValue(std::in_place, std::forward<Args>(args)...) {

        }

    private:
        Property<T>* m_target;
        std::optional<T> m_fixedValue;

        friend Observer<T>;
    };

    class ObserverBase {
    public:
        ObserverBase(Component* owner);
        ObserverBase(ObserverBase&&) noexcept;
        ObserverBase& operator=(ObserverBase&&);
        virtual ~ObserverBase();

        ObserverBase(const ObserverBase&) = delete;
        ObserverBase& operator=(const ObserverBase&&) = delete;

    protected:
        Component* owner() noexcept;
        const Component* owner() const noexcept;

    private:
        Component* m_owner;

        void addSelfTo(Component*);
        void removeSelfFrom(Component*);

        friend Component;
    };

    template<typename T>
    class Observer : public ObserverBase {
    public:
        template<typename ComponentType>
        Observer(ComponentType* self, void (ComponentType::* onUpdate)(DiffArgType<T>), Property<T>& target)
            : ObserverBase(self)
            , m_target(&target)
            , m_fixedValue(std::nullopt)
            , m_onUpdate(makeUpdateFunction(self, onUpdate)) {
            m_target->m_observers.push_back(this);
        }
        template<typename ComponentType>
        Observer(ComponentType* self, void (ComponentType::* onUpdate)(DiffArgType<T>), T fixedValue)
            : ObserverBase(self)
            , m_target(nullptr)
            , m_fixedValue(std::move(fixedValue))
            , m_onUpdate(makeUpdateFunction(self, onUpdate)) {

        }
        template<typename ComponentType>
        Observer(ComponentType* self, void (ComponentType::* onUpdate)(DiffArgType<T>), PropertyOrValue<T> pv)
            : ObserverBase(self)
            , m_target(nullptr)
            , m_fixedValue(std::nullopt)
            , m_onUpdate(makeUpdateFunction(self, onUpdate)) {
            assign(pv);
        }
        template<typename ComponentType>
        Observer(ComponentType* self, void (ComponentType::* onUpdate)(DiffArgType<T>))
            : ObserverBase(self)
            , m_target(nullptr)
            , m_fixedValue(std::nullopt)
            , m_onUpdate(makeUpdateFunction(self, onUpdate)) {

        }

        Observer(Observer&& o) noexcept 
            : ObserverBase(std::move(o))
            , m_target(std::exchange(o.m_target, nullptr))
            , m_fixedValue(std::exchange(o.m_fixedValue, std::nullopt))
            , m_onUpdate(std::exchange(o.m_onUpdate, nullptr)) {
            if (m_target) {
                auto& v = m_target->m_observers;
                assert(std::count(v.begin(), v.end(), &o) == 1);
                assert(std::count(v.begin(), v.end(), this) == 0);
                *std::find(v.begin(), v.end(), &o) = this;
            }
        }
        Observer& operator=(Observer&& o) {
            if (&o == this) {
                return *this;
            }
            reset();
            m_target = std::exchange(o.m_target, nullptr);
            m_fixedValue = std::exchange(o.m_fixedValue, std::nullopt);
            m_onUpdate = std::exchange(o.m_onUpdate, nullptr);
            if (m_target) {
                auto& v = m_target->m_observers;
                assert(std::count(v.begin(), v.end(), &o) == 1);
                assert(std::count(v.begin(), v.end(), this) == 0);
                *std::find(v.begin(), v.end(), &o) = this;
            }
            return *this;
        }
        ~Observer() {
            reset();
        }

        Observer(const Observer&) = delete;
        Observer& operator=(const Observer&) = delete;

        void assign(Property<T>& target) {
            m_target = &target;
            m_fixedValue = std::nullopt;

            const auto& init = T{};
            // if target is dirty, bring this observer up to speed with its previous
            // value so that next update purge will be accurate.
            // Otherwise, if target is clean, bring this observer up to speed with
            // its current value.
            const auto diff = Difference<T>::compute(
                init,
                target.m_previousValue.has_value() ?
                static_cast<const T&>(*target.m_previousValue) :
                static_cast<const T&>(target.m_value)
            );
            update(diff);
            m_target->m_observers.push_back(this);
        }

        void assign(T fixedValue) {
            m_target = nullptr;
            const auto diff = Difference<T>::compute(
                static_cast<const T&>(*m_fixedValue),
                static_cast<const T&>(fixedValue)
            );
            m_fixedValue = std::move(fixedValue);
            update(diff);
        }

        void assign(const PropertyOrValue<T>& pv) {
            assert(pv.m_fixedValue.has_value() || pv.m_target);
            if (pv.m_target) {
                assign(*pv.m_target);
            } else {
                assign(static_cast<const T&>(*pv.m_fixedValue));
            }
        }

        bool hasTarget() const noexcept {
            return m_target;
        }

        bool hasFixedValue() const noexcept {
            return m_fixedValue.has_value();
        }

        bool hasValue() const noexcept {
            return hasTarget() || hasFixedValue();
        }

        const T& getValueOnce() const noexcept {
            assert(hasValue());
            if (m_target) {
                return m_target->getOnce();
            } else {
                return *m_fixedValue;
            }
        }

        Property<T>* target() noexcept {
            return m_target;
        }
        const Property<T>* target() const noexcept {
            return m_target;
        }

        void reset() {
            if (m_target) {
                auto& v = m_target->m_observers;
                assert(std::count(v.begin(), v.end(), this) == 1);
                auto it = std::find(v.begin(), v.end(), this);
                assert(it != v.end());
                v.erase(it);
            }
            m_target = nullptr;
            m_fixedValue.reset();
        }

        void update(DiffArgType<T> t) {
            assert(m_onUpdate);
            assert(owner());
            m_onUpdate(owner(), t);
        }

    private:
        Property<T>* m_target;
        std::optional<T> m_fixedValue;
        std::function<void(Component*, DiffArgType<T>)> m_onUpdate;

        template<typename ComponentType>
        static std::function<void(Component*, DiffArgType<T>)> makeUpdateFunction(ComponentType* self, void (ComponentType::* onUpdate)(DiffArgType<T>)) {
            static_assert(std::is_base_of_v<Component, ComponentType>, "ComponentType must derive from Component");
            // NOTE: self could be captured here, but is not, because it will become a
            // dangling pointer if the component is moved, which it will be anytime
            // it's passed to an AnyComponent
            return [onUpdate](Component* self, DiffArgType<T> t){
                assert(self);
                assert(dynamic_cast<ComponentType*>(self));
                auto sd = static_cast<ComponentType*>(self);
                if (sd->isMounted()) {
                    (sd->*onUpdate)(t);
                }
            };
        }

        friend Property<T>;
    };

    // Property that is a pure function of another property.
    // Useful for 
    // TODO: inheriting from Property<T> doesn't really make sense,
    // in terms of inheriting set() and m_value. If those could be moved
    // into a sibling class like PrimaryProperty (subject to some all-around
    // renaming), this could be neatly resolved.
    template<typename T, typename U>
    class DerivedProperty : public Property<T>, public Observer<U> {
    public:
        DerivedProperty(Property<U>&, std::function<T(const U&)>);
    };

} // namespace ui
