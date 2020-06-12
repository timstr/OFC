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


    template<typename T>
    class ListOfEdits {
    public:
        ListOfEdits(const std::vector<T>& vecOld, const std::vector<T>& vecNew)
            : m_oldValue(vecOld)
            , m_newValue(vecNew) {
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

        const std::vector<T>& oldValue() const noexcept {
            return m_oldValue;
        }

        const std::vector<T>& newValue() const noexcept {
            return m_newValue;
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
        const std::vector<T>& m_oldValue;
        const std::vector<T>& m_newValue;
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

        static std::decay_t<T> computeFirst(const T& vNew) noexcept {
            return vNew;
        }
    };

    template<typename T>
    struct Difference<std::vector<T>> {
        using ArgType = const ListOfEdits<T>&;

        static ListOfEdits<T> compute(const std::vector<T>& vOld, const std::vector<T>& vNew) noexcept {
            return ListOfEdits<T>{vOld, vNew};
        }

        static ListOfEdits<T> computeFirst(const std::vector<T>& vNew) noexcept {
            return ListOfEdits<T>{std::vector<T>{}, vNew};
        }
    };

    template<typename T>
    using DiffArgType = typename Difference<T>::ArgType;

    template<typename T>
    class Property;

    template<typename T>
    class Observer;

    // Base class of both Property and DerivedProperty
    template<typename T>
    class PropertyBase {
    public:
        PropertyBase() noexcept = default;
        PropertyBase(const T& t)
            : m_value(t)
            , m_previousValue(std::nullopt) {

        }
        PropertyBase(T&& t)
            : m_value(std::move(t))
            , m_previousValue(std::nullopt) {

        }
        ~PropertyBase() noexcept {
            for (auto& o : m_observers) {
                o->reset();
            }
        }

        PropertyBase(const PropertyBase&) = delete;
        PropertyBase(PropertyBase&&) = delete;
        PropertyBase& operator=(PropertyBase&&) = delete;
        PropertyBase& operator=(const PropertyBase&) = delete;

        const T& getOnce() const noexcept {
            return m_value;
        }

    protected:

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

        T& getOnceMutable() noexcept {
            if (!m_previousValue.has_value()) {
                m_previousValue.emplace(static_cast<const T&>(m_value));
                registerForUpdate();
            }
            return m_value;
        }

    private:
        T m_value;
        std::optional<T> m_previousValue;
        mutable std::vector<Observer<T>*> m_observers;

        void purgeUpdates() {
            if (!m_previousValue.has_value()) {
                return;
            }
            if (*m_previousValue == m_value) {
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
    class Property : public PropertyBase<T> {
    public:
        explicit Property(const T& t) : PropertyBase<T>(t) {

        }
        explicit Property(T&& t) : PropertyBase<T>(std::move(t)) {

        }


        using PropertyBase::getOnce;
        using PropertyBase::getOnceMutable;
        using PropertyBase::set;

        // TODO: operator= for const T& / T&&?

    private:
    };

    template<typename T>
    class PropertyOrValue final {
    public:
        // PropertyOrValue is implictly constructible from an l-value reference
        // to a matching property, in which case it will point to that property
        // and contain no fixed value.
        PropertyOrValue(const PropertyBase<T>& target) noexcept
            : m_target(&target)
            , m_fixedValue(std::nullopt) {

        }

        // PropertyOrValue is implicitly constructible from anything that can be
        // used to construct a value of type T, in which case it will not point
        // to any property and will instead contain a fixed value.
        template<
            typename... Args,
            std::enable_if_t<
            !std::is_same_v<const PropertyBase<T>&, Args...> &&
            std::is_constructible_v<T, Args...>
            >* = nullptr
        >
        PropertyOrValue(Args&&... args)
            : m_target(nullptr)
            , m_fixedValue(std::in_place, std::forward<Args>(args)...) {

        }

        PropertyOrValue(PropertyOrValue&& p) noexcept
            : m_target(std::exchange(p.m_target, nullptr))
            , m_fixedValue(std::move(p.m_fixedValue)) {

        }

        PropertyOrValue(const PropertyOrValue&) = delete;

        ~PropertyOrValue() noexcept = default;

        PropertyOrValue& operator=(PropertyOrValue&& p) noexcept {
            m_target = std::exchange(p.m_target, nullptr);
            m_fixedValue = std::move(p.m_fixedValue);
        }

        PropertyOrValue& operator=(const PropertyOrValue&) = delete;

        const T& getOnce() noexcept {
            assert((m_target == nullptr) == m_fixedValue.has_value());
            if (m_target) {
                return m_target->getOnce();
            } else {
                return *m_fixedValue;
            }
        }

    private:
        const PropertyBase<T>* m_target;
        std::optional<T> m_fixedValue;

        friend Observer<T>;
    };

    class ObserverBase;

    class ObserverOwner {
    public:
        ObserverOwner() noexcept = default;
        ObserverOwner(ObserverOwner&&) noexcept;
        virtual ~ObserverOwner() noexcept;

        ObserverOwner(const ObserverOwner&) = delete;
        ObserverOwner& operator=(ObserverOwner&&) = delete;
        ObserverOwner& operator=(const ObserverOwner&) = delete;

    private:
        std::vector<ObserverBase*> m_ownObservers;

        friend ObserverBase;
    };

    class ObserverBase {
    public:
        ObserverBase(ObserverOwner* owner);
        ObserverBase(ObserverBase&&) noexcept;
        ObserverBase& operator=(ObserverBase&&);
        virtual ~ObserverBase();

        ObserverBase(const ObserverBase&) = delete;
        ObserverBase& operator=(const ObserverBase&&) = delete;

    protected:
        ObserverOwner* owner() noexcept;
        const ObserverOwner* owner() const noexcept;

    private:
        ObserverOwner* m_owner;

        void addSelfTo(ObserverOwner*);
        void removeSelfFrom(ObserverOwner*);

        friend ObserverOwner;
    };

    template<typename T>
    class Observer : public ObserverBase {
    public:
        template<typename ObserverOwnerType>
        Observer(ObserverOwnerType* self, void (ObserverOwnerType::* onUpdate)(DiffArgType<T>), const PropertyBase<T>& target)
            : ObserverBase(self)
            , m_target(&target)
            , m_fixedValue(std::nullopt)
            , m_onUpdate(makeUpdateFunction(self, onUpdate)) {
            m_target->m_observers.push_back(this);
        }
        template<typename ObserverOwnerType>
        Observer(ObserverOwnerType* self, void (ObserverOwnerType::* onUpdate)(DiffArgType<T>), T fixedValue)
            : ObserverBase(self)
            , m_target(nullptr)
            , m_fixedValue(std::move(fixedValue))
            , m_onUpdate(makeUpdateFunction(self, onUpdate)) {

        }
        template<typename ObserverOwnerType>
        Observer(ObserverOwnerType* self, void (ObserverOwnerType::* onUpdate)(DiffArgType<T>), PropertyOrValue<T> pv)
            : ObserverBase(self)
            , m_target(nullptr)
            , m_fixedValue(std::nullopt)
            , m_onUpdate(makeUpdateFunction(self, onUpdate)) {
            assign(pv);
        }
        template<typename ObserverOwnerType>
        Observer(ObserverOwnerType* self, void (ObserverOwnerType::* onUpdate)(DiffArgType<T>))
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

        void assign(const PropertyBase<T>& target) {
            m_target = &target;
            m_fixedValue = std::nullopt;

            // if target is dirty, bring this observer up to speed with its previous
            // value so that next update purge will be accurate.
            // Otherwise, if target is clean, bring this observer up to speed with
            // its current value.
            const auto diff = Difference<T>::computeFirst(
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
        const PropertyBase<T>* m_target;
        std::optional<T> m_fixedValue;
        std::function<void(ObserverOwner*, DiffArgType<T>)> m_onUpdate;

        template<typename ObserverOwnerType>
        static std::function<void(ObserverOwner*, DiffArgType<T>)> makeUpdateFunction(ObserverOwnerType* self, void (ObserverOwnerType::* onUpdate)(DiffArgType<T>)) {
            static_assert(std::is_base_of_v<ObserverOwner, ObserverOwnerType>, "ObserverOwnerType must derive from ObserverOwner");
            // NOTE: self could be captured here, but is not, because it will become a
            // dangling pointer if the ObserverOwner is moved, which will be often in the
            // case of Components being passed stored inside AnyComponent
            return [onUpdate](ObserverOwner* self, DiffArgType<T> t){
                assert(self);
                assert(dynamic_cast<ObserverOwnerType*>(self));
                auto sd = static_cast<ObserverOwnerType*>(self);
                if constexpr (std::is_base_of_v<Component, ObserverOwnerType>) {
                    const auto mounted = static_cast<Component*>(self)->isMounted();
                    if (!mounted) {
                        return;
                    }
                }
                (sd->*onUpdate)(t);
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
    class DerivedProperty : public PropertyBase<T>, public ObserverOwner {
    public:
        DerivedProperty(PropertyOrValue<U> p, std::function<T(DiffArgType<U>)> f)
            : PropertyBase<T>(f(Difference<U>::computeFirst(p.getOnce())))
            , m_fn(std::move(f))
            , m_observer(this, &DerivedProperty::onUpdate, std::move(p)) {

        }

    private:
        std::function<T(DiffArgType<U>)> m_fn;
        Observer<U> m_observer;

        void onUpdate(DiffArgType<U> u) {
            assert(m_fn);
            PropertyBase<T>::set(m_fn(u));
        }
    };

} // namespace ui
