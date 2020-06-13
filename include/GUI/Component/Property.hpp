#pragma once

#include <cassert>
#include <functional>
#include <memory>
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
        PropertyBase(T&& t) noexcept
            : m_value(std::move(t))
            , m_previousValue(std::nullopt) {

        }
        PropertyBase(PropertyBase&& pb) noexcept
            : m_value(std::move(pb.m_value))
            , m_previousValue(std::move(pb.m_previousValue))
            , m_observers(std::move(pb.m_observers)) {

            for (auto& o : m_observers) {
                assert(o->getProperty() == &pb);
                o->assign(*this);
            }
        }
        virtual ~PropertyBase() noexcept {
            for (auto& o : m_observers) {
                o->reset();
            }
        }

        PropertyBase(const PropertyBase&) = delete;
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
        explicit PropertyOrValue() noexcept
            : m_targetProperty(nullptr)
            , m_ownProperty(nullptr)
            , m_fixedValue(std::nullopt) {

        }

        // PropertyOrValue is implictly constructible from a reference
        // to a matching property, in which case it will point to that property
        // and contain no fixed value.
        PropertyOrValue(const PropertyBase<T>& target) noexcept
            : m_targetProperty(&target)
            , m_ownProperty(nullptr)
            , m_fixedValue(std::nullopt) {

        }

        // PropertyOrValue is implicitly constructible from an r-value reference to
        // a derived property, in which case that property is moved from and ownership
        // is taken.
        template<
            typename DerivedPropertyType,
            typename std::enable_if_t<
                std::is_base_of_v<PropertyBase<T>, std::decay_t<DerivedPropertyType>> &&
                !std::is_same_v<std::decay_t<DerivedPropertyType>, Property<T>>
            >* = nullptr
        >
        PropertyOrValue(DerivedPropertyType&& derivedProperty) noexcept
            : m_targetProperty(nullptr)
            , m_ownProperty(std::make_unique<std::decay_t<DerivedPropertyType>>(std::move(derivedProperty)))
            , m_fixedValue(std::nullopt) {

        }

        // PropertyOrValue is implicitly constructible from anything that can be
        // used to construct a value of type T, in which case it will not point
        // to any property and will instead contain a fixed value.
        template<
            typename... Args,
            std::enable_if_t<
                (sizeof...(Args) > 0) &&
                !std::is_same_v<const PropertyBase<T>&, Args...> &&
                std::is_constructible_v<T, Args...>
            >* = nullptr
        >
        PropertyOrValue(Args&&... args)
            : m_targetProperty(nullptr)
            , m_ownProperty(nullptr)
            , m_fixedValue(std::in_place, std::forward<Args>(args)...) {

        }

        PropertyOrValue(PropertyOrValue&& p) noexcept
            : m_targetProperty(std::exchange(p.m_targetProperty, nullptr))
            , m_ownProperty(std::move(p.m_ownProperty))
            , m_fixedValue(std::move(p.m_fixedValue)) {
            assert(!hasProperty() || !hasFixedValue());
            assert(!hasProperty() || (hasTargetProperty() != hasOwnProperty()));
        }

        PropertyOrValue(const PropertyOrValue&) = delete;

        ~PropertyOrValue() noexcept = default;

        PropertyOrValue& operator=(PropertyOrValue&& p) noexcept {
            if (&p == this) {
                return *this;
            }
            assert(!p.hasProperty() || !p.hasFixedValue());
            assert(!p.hasProperty() || (p.hasTargetProperty() != p.hasOwnProperty()));
            assert(!hasProperty() || !hasFixedValue());
            assert(!hasProperty() || (hasTargetProperty() != hasOwnProperty()));
            m_targetProperty = std::exchange(p.m_targetProperty, nullptr);
            m_ownProperty = std::move(p.m_ownProperty);
            m_fixedValue = std::move(p.m_fixedValue);
            assert(!hasProperty() || !hasFixedValue());
            assert(!hasProperty() || (hasTargetProperty() != hasOwnProperty()));
            return *this;
        }

        PropertyOrValue& operator=(const PropertyOrValue&) = delete;

        bool hasTargetProperty() const noexcept {
            return m_targetProperty != nullptr;
        }

        bool hasOwnProperty() const noexcept {
            return m_ownProperty != nullptr;
        }

        bool hasProperty() const noexcept {
            return hasTargetProperty() || hasOwnProperty();
        }

        bool hasFixedValue() const noexcept {
            return m_fixedValue.has_value();
        }

        bool hasValue() const noexcept {
            assert(!hasProperty() || !hasFixedValue());
            assert(!hasProperty() || (hasTargetProperty() != hasOwnProperty()));
            return hasProperty() || hasFixedValue();
        }

        PropertyBase<T>* getProperty() noexcept {
            return const_cast<PropertyBase<T>*>(const_cast<const PropertyOrValue<T>*>(this)->getProperty());
        }
        const PropertyBase<T>* getProperty() const noexcept {
            assert(!hasProperty() || !hasFixedValue());
            assert(!hasProperty() || (hasTargetProperty() != hasOwnProperty()));
            if (m_targetProperty) {
                return m_targetProperty;
            }
            return m_ownProperty.get();
        }

        const T& getValueOnce() const noexcept {
            assert(!hasProperty() || !hasFixedValue());
            assert(!hasProperty() || (hasTargetProperty() != hasOwnProperty()));
            assert(hasValue());
            if (m_targetProperty) {
                return m_targetProperty->getOnce();
            } else if (m_ownProperty) {
                return m_ownProperty->getOnce();
            } else {
                assert(m_fixedValue.has_value());
                return *m_fixedValue;
            }
        }

        void reset() {
            assert(!hasProperty() || !hasFixedValue());
            assert(!hasProperty() || (hasTargetProperty() != hasOwnProperty()));
            m_targetProperty = nullptr;
            m_ownProperty = nullptr;
            m_fixedValue.reset();
        }

    private:
        const PropertyBase<T>* m_targetProperty;
        std::unique_ptr<PropertyBase<T>> m_ownProperty;
        std::optional<T> m_fixedValue;
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
        Observer(ObserverOwnerType* self, void (ObserverOwnerType::* onUpdate)(DiffArgType<T>), PropertyOrValue<T> pv)
            : ObserverBase(self)
            , m_propertyOrValue(std::move(pv))
            , m_onUpdate(makeUpdateFunction(self, onUpdate)) {

            if (auto p = m_propertyOrValue.getProperty()) {
                auto& v = p->m_observers;
                assert(std::count(v.begin(), v.end(), this) == 0);
                v.push_back(this);
            }
        }
        template<typename ObserverOwnerType>
        Observer(ObserverOwnerType* self, void (ObserverOwnerType::* onUpdate)(DiffArgType<T>))
            : ObserverBase(self)
            , m_propertyOrValue()
            , m_onUpdate(makeUpdateFunction(self, onUpdate)) {

        }

        Observer(Observer&& o) noexcept 
            : ObserverBase(std::move(o))
            , m_propertyOrValue(std::move(o.m_propertyOrValue))
            , m_onUpdate(std::exchange(o.m_onUpdate, nullptr)) {
            if (auto p = m_propertyOrValue.getProperty()) {
                auto& v = p->m_observers;
                assert(std::count(v.begin(), v.end(), &o) == 1);
                assert(std::count(v.begin(), v.end(), this) == 0);
                auto it = std::find(v.begin(), v.end(), &o);
                assert(it != v.end());
                *it = this;
            }
        }
        Observer& operator=(Observer&& o) {
            if (&o == this) {
                return *this;
            }
            reset();
            m_propertyOrValue = std::move(o.m_propertyOrValue);
            m_onUpdate = std::exchange(o.m_onUpdate, nullptr);
            if (auto p = m_propertyOrValue.getProperty()) {
                auto& v = p->m_observers;
                assert(std::count(v.begin(), v.end(), &o) == 1);
                assert(std::count(v.begin(), v.end(), this) == 0);
                auto it = std::find(v.begin(), v.end(), &o);
                assert(it != v.end());
                *it = this;
            }
            return *this;
        }
        ~Observer() {
            reset();
        }

        Observer(const Observer&) = delete;
        Observer& operator=(const Observer&) = delete;

        void assign(const PropertyBase<T>& target) {
            // if target is dirty, bring this observer up to speed with its previous
            // value so that next update purge will be accurate.
            // Otherwise, if target is clean, bring this observer up to speed with
            // its current value
            const T& newValue = target.m_previousValue.has_value() ?
                static_cast<const T&>(*target.m_previousValue) :
                static_cast<const T&>(target.m_value);

            const auto diff = [&] {
                if (m_propertyOrValue.hasValue()) {
                    return Difference<T>::compute(
                        m_propertyOrValue.getValueOnce(),
                        newValue
                    );
                } else {
                    return Difference<T>::computeFirst(newValue);
                }
            }();
            reset();
            assert(!m_propertyOrValue.hasValue());
            m_propertyOrValue = target;
            assert(m_propertyOrValue.hasTargetProperty());
            assert(!m_propertyOrValue.hasOwnProperty());
            assert(!m_propertyOrValue.hasFixedValue());
            update(diff);
            auto p = m_propertyOrValue.getProperty();
            assert(p);
            auto& v = p->m_observers;
            assert(std::count(v.begin(), v.end(), this) == 0);
            v.push_back(this);
        }

        void assign(T fixedValue) {
            const auto diff = [&] {
                if (m_propertyOrValue.hasValue()) {
                    return Difference<T>::compute(
                        m_propertyOrValue.getValueOnce(),
                        static_cast<const T&>(fixedValue)
                    );
                } else {
                    return Difference<T>::computeFirst(static_cast<const T&>(fixedValue));
                }
            }();
            reset();
            assert(!m_propertyOrValue.hasValue());

            m_propertyOrValue = std::move(fixedValue);
            update(diff);
            assert(m_propertyOrValue.hasFixedValue());
            assert(!m_propertyOrValue.hasTargetProperty());
            assert(!m_propertyOrValue.hasOwnProperty());
        }

        // NOTE: if the PropertyOrValue that is passed owns a property, this Observer does NOT
        // take ownership of that property, and instead stores a reference to it.
        // This behaviour is needed for ContextProvider and ContextConsumer to work as expected.
        void assign(const PropertyOrValue<T>& pv) {
            assert(pv.hasValue());
            if (auto p = pv.getProperty()) {
                assign(*p);
            } else {
                assert(pv.hasFixedValue());
                assign(pv.getValueOnce());
            }
        }

        bool hasValue() const noexcept {
            return m_propertyOrValue.hasValue();
        }

        bool hasFixedValue() const noexcept {
            return m_propertyOrValue.hasFixedValue();
        }

        const PropertyBase<T>* getProperty() const noexcept {
            return m_propertyOrValue.getProperty();
        }

        const T& getValueOnce() const noexcept {
            return m_propertyOrValue.getValueOnce();
        }

        void reset() {
            if (auto p = m_propertyOrValue.getProperty()) {
                auto& v = p->m_observers;
                assert(std::count(v.begin(), v.end(), this) == 1);
                auto it = std::find(v.begin(), v.end(), this);
                assert(it != v.end());
                v.erase(it);
            }
            m_propertyOrValue.reset();
        }

        void update(DiffArgType<T> t) {
            assert(m_onUpdate);
            assert(owner());
            m_onUpdate(owner(), t);
        }

    private:
        PropertyOrValue<T> m_propertyOrValue;

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

    // Property that is a pure function of another property
    template<typename T, typename U>
    class DerivedProperty : public PropertyBase<T>, public ObserverOwner {
    public:
        DerivedProperty(std::function<T(DiffArgType<U>)> f, PropertyOrValue<U> p)
            : PropertyBase<T>(f(Difference<U>::computeFirst(p.getValueOnce())))
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

    template<typename U, std::size_t Index, typename Derived>
    class DerivedPropertyArgumentImpl {
        DerivedPropertyArgumentImpl(PropertyOrValue<U> p)
            : m_observer(static_cast<Derived*>(this), &DerivedPropertyArgumentImpl<U, Index, Derived>::onUpdate, std::move(p)) {

        }

    private:
        Observer<U> m_observer;

        void onUpdate(DiffArgType<U> u) {
            static_cast<Derived*>(this)->updateFrom<U, Index>(u);
        }
    };

    // Property that is a pure function of two or more properties
    // TODO: can this be made more efficient when multiple input
    // properties have changed?
    /*template<typename T, typename U1, typename U2, typename... URest>
    class DerivedProperty
        : public PropertyBase<T>
        , public ObserverOwner
        , private DerivedPropertyArgumentImpl<U1, sizeof...(URest) + 1, DerivedProperty>
        , private DerivedPropertyArgumentImpl<U2, sizeof...(URest), DerivedProperty>
        , private DerivedPropertyArgumentImpl<URest, sizeof...(URest), DerivedProperty>... {

    private:
        template<typename U, std::size_t Index>
        void updateFrom(DiffArgType<U> u) {
            // TODO
        }
    };*/

} // namespace ui
