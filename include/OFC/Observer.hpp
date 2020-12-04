#pragma once

#include <cassert>
#include <functional>
#include <memory>
#include <optional>
#include <vector>

namespace ofc {
    
    template<typename T>
    class Value;

    template<typename T>
    class Valuelike;

    template<typename T>
    class Observer;

    namespace detail {
        void enqueueValueUpdater(std::function<void()>);

        void updateAllValues();

    } // namespace detail


    template<typename T>
    using CRefOrValue = std::conditional_t<
        std::is_scalar_v<std::decay_t<T>>,
        std::decay_t<T>,
        const std::decay_t<T>&
    >;

    // Summary<T> is used to describe the previous contents of a Value<T>.
    // This is necessary for non-copyable types, and could be used for
    // improving space efficiency on large data types (using e.g. a hash).
    // Every summary type must support operator== and operator!= for comparison.
    // This type is only intended for internal use. The most that client code
    // should need to know is how to specialize this class for custom types
    // For trivial types, the summary is the same type
    template<typename T>
    struct Summary {
        static_assert(std::is_copy_constructible_v<T>);

        using Type = std::decay_t<T>;

        static Type compute(const T& t){
            return t;
        }
    };

    // For Value, the summary is simply the address of the underlying implementation.
    // This is because changes to the contents of the Value<T> are
    // not tracked at this level, because things can already subscribe
    // to those directly. The address still provides a unique way of
    // referring to Values.
    template<typename T>
    struct Summary<Value<T>> {
        using Type = const void*;

        static Type compute(const Value<T>& v) {
            return v.summarize();
        }
    };

    // For vectors, the summary is a vector of summaries
    template<typename T>
    struct Summary<std::vector<T>> {
        using Type = std::vector<typename Summary<T>::Type>;

        static Type compute(const std::vector<T>& v) {
            auto out = Type{};
            for (const auto& t : v) {
                out.push_back(Summary<T>::compute(t));
            }
            return out;
        }
    };

    // For pair<T1, T2>, the summary is a pair of summaries
    template<typename T1, typename T2>
    struct Summary<std::pair<T1, T2>> {
        using Type = std::pair<
            typename Summary<T1>::Type,
            typename Summary<T2>::Type
        >;

        static Type compute(const std::pair<T1, T2>& p) {
            return {
                Summary<T1>::compute(p.first),
                Summary<T2>::compute(p.second)
            };
        }
    };

    // tuple<Ts...> is similar to pair
    template<typename... Ts>
    struct Summary<std::tuple<Ts...>> {
        using Type = std::tuple<typename Summary<Ts>::Type...>;

        static Type compute(const std::tuple<Ts...>& t) {
            return computeImpl(t, std::make_index_sequence<sizeof...(Ts)>{});
        }

    private:
        template<std::size_t... Indices>
        static Type computeImpl(const std::tuple<Ts...>& t, std::index_sequence<Indices...> /* unused */) {
            return {
                Summary<Ts>::compute(std::get<Indices>(t))...
            };
        }
    };

    // TODO: variant, optional, etc

    // For unique_ptr, the summary is a raw pointer
    // This avoids the problem of copying unique pointers
    // to keep track of past state.
    template<typename T>
    struct Summary<std::unique_ptr<T>> {
        using Type = T*;

        static Type compute(const std::unique_ptr<T>& p) {
            return p.get();
        }
    };


    template<typename T>
    using SummaryType = typename Summary<T>::Type;

    // Difference<T> is used to represent changes between subsequent contents
    // in a Value<T>. For trivial types, the difference is merely the new value.
    // For vectors, the difference is a list of edits (a diff).
    template<typename T>
    struct Difference {
        // For scalar types, the difference argument type is simply that type with all top-level
        // CV-qualifications removed. This saves users from having to type the arguments of update
        // functions for simple pointers as `const SomeType* const&` when `const SomeType*` suffices
        // For other, more complicated types, the difference argument type is a reference to const T
        using ArgType = CRefOrValue<SummaryType<T>>;

        static SummaryType<T> compute(const SummaryType<T>& /* vOld */, const T& vNew) noexcept {
            return Summary<T>::compute(vNew);
        }

        static SummaryType<T> computeFirst(const T& vNew) noexcept {
            return Summary<T>::compute(vNew);
        }
    };

    template<typename T>
    using DiffArgType = typename Difference<T>::ArgType;

    template<typename T>
    class ListOfEdits {
    public:
        ListOfEdits(const std::vector<SummaryType<T>>& vecOld, const std::vector<T>& vec)
            : m_oldValue(vecOld)
            , m_newValue(vec) {
            // Compute longest common subsequence using dynamic-programming table
            auto vecNew = Summary<std::vector<T>>::compute(vec);
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
                        edits.push_back(Edit(EditType::Insertion, &vec[newBegin + j - 1]));
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

        const std::vector<Summary<T>>& oldValue() const noexcept {
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
                assert(m_value);
                return *m_value;
            }

        private:
            Edit(EditType type, const T* value = nullptr)
                : m_type(type)
                , m_value(value) {

            }

            const EditType m_type;
            const T* const m_value;

            friend ListOfEdits<T>;
        };

    private:
        std::vector<SummaryType<T>> m_oldValue;
        const std::vector<T>& m_newValue;
        std::vector<Edit> m_edits;
    };

    template<typename T>
    struct Difference<std::vector<T>> {
        using ArgType = const ListOfEdits<T>&;

        static ListOfEdits<T> compute(const std::vector<SummaryType<T>>& vOld, const std::vector<T>& vNew) noexcept {
            return ListOfEdits<T>{vOld, vNew};
        }

        static ListOfEdits<T> computeFirst(const std::vector<T>& vNew) noexcept {
            return ListOfEdits<T>{
                std::vector<SummaryType<T>>{},
                vNew
            };
        }
    };


    template<typename T>
    class ValueImpl {
    public:
        ValueImpl() noexcept
            : m_value{}
            , m_previousValue(std::nullopt) {
        
        }
        ValueImpl(const T& t)
            : m_value(t)
            , m_previousValue(std::nullopt) {

        }
        ValueImpl(T&& t) noexcept
            : m_value(std::move(t))
            , m_previousValue(std::nullopt) {

        }

        ValueImpl(const ValueImpl&) = delete;
        ValueImpl(ValueImpl&&) = delete;

        virtual ~ValueImpl() noexcept {
            for (auto& o : m_observers) {
                o->reset();
            }
        }


        ValueImpl& operator=(const ValueImpl&) = delete;
        ValueImpl& operator=(ValueImpl&&) = delete;

        const T& getOnce() const noexcept {
            return m_value;
        }

        template<typename F>
        auto map(F&& f) const {
            using R = std::invoke_result_t<std::decay_t<F>, DiffArgType<T>>;
            static_assert(!std::is_void_v<R>);
            static_assert(std::is_base_of_v<ValueImpl<R>, DerivedValueImpl<R, T>>);
            return Value<R>{std::make_unique<DerivedValueImpl<R, T>>(
                std::forward<F>(f),
                Valuelike<T>(this)
            )};
        }

        void set(const T& t) {
            if (!m_previousValue.has_value()) {
                m_previousValue.emplace(summarize());
                registerForUpdate();
            }
            m_value = t;
        }
        void set(T&& t) {
            if (!m_previousValue.has_value()) {
                m_previousValue.emplace(summarize());
                registerForUpdate();
            }
            m_value = std::move(t);
        }

        T& getOnceMut() noexcept {
            if (!m_previousValue.has_value()) {
                m_previousValue.emplace(summarize());
                registerForUpdate();
            }
            return m_value;
        }

    private:
        T m_value;
        std::optional<SummaryType<T>> m_previousValue;
        mutable std::vector<Observer<T>*> m_observers;

        SummaryType<T> summarize() const {
            auto result = Summary<T>::compute(static_cast<const T&>(m_value));
            static_assert(std::is_same_v<SummaryType<T>, decltype(result)>);
            return result;
        }

        void purgeUpdates() {
            if (!m_previousValue.has_value()) {
                return;
            }
            if (*m_previousValue == summarize()) {
                m_previousValue.reset();
                return;
            }
            const auto diff = Difference<T>::compute(
                static_cast<const SummaryType<T>&>(*m_previousValue),
                static_cast<const T&>(m_value)
            );
            m_previousValue.reset();
            for (auto& o : m_observers) {
                o->update(static_cast<const DiffArgType<T>&>(diff));
            }
        }

        void registerForUpdate() {
            detail::enqueueValueUpdater([this]() {
                purgeUpdates();
            });
        }

        friend Observer<T>;
    };

    template<typename T>
    class Value {
    public:
        Value()
            : m_impl(std::make_unique<ValueImpl<T>>()) {
        
        }

        explicit Value(const T& t)
            : m_impl(std::make_unique<ValueImpl<T>>(t)) {

        }
        explicit Value(T&& t)
            : m_impl(std::make_unique<ValueImpl<T>>(std::move(t))) {

        }

        Value(Value&& v) noexcept
            : m_impl(std::move(v.m_impl)) {
        
        }
        Value(const Value&) = delete;

        Value& operator=(Value&& v) noexcept {
            m_impl = std::move(v.m_impl);
            return *this;
        }
        Value& operator=(const Value&) = delete;

        const T& getOnce() const noexcept {
            assert(m_impl);
            return m_impl->getOnce();
        }

        T& getOnceMut() noexcept {
            assert(m_impl);
            return m_impl->getOnceMut();
        }
        
        void set(const T& t) {
            assert(m_impl);
            m_impl->set(t);
        }

        void set(T&& t) {
            assert(m_impl);
            m_impl->set(std::move(t));
        }

        // This function serves to enable "interior mutability" as used in Rust,
        // for example, when you just really need to turn a const reference into
        // a non-const reference. To be used with caution.
        Value& makeMutable() const noexcept {
            return const_cast<Value&>(*this);
        }

        template<typename F>
        auto map(F&& f) const {
            assert(m_impl);
            return m_impl->map(std::forward<F>(f));
        }

    private:
        std::unique_ptr<ValueImpl<T>> m_impl;

        Value(std::unique_ptr<ValueImpl<T>> impl) noexcept
            : m_impl(std::move(impl)) {
        
        }

        const void* summarize() const noexcept {
            return static_cast<const void*>(m_impl.get());
        }

        template<typename TT>
        friend class ValueImpl;

        friend Valuelike<T>;

        template<typename TT, typename... Rest>
        friend class CombinedValues;

        friend Summary<Value<T>>;
    };

    template<typename T, typename... Rest>
    class CombinedValues final {
    public:
        CombinedValues(Valuelike<T>&& t, Valuelike<Rest>&&... rest) 
            : m_values(std::move(t), std::move(rest)...) {

        }

        template<typename F>
        auto map(F&& f) && noexcept {
            using R = std::invoke_result_t<std::decay_t<F>, DiffArgType<T>, DiffArgType<Rest>...>;
            return mapImpl<F, R>(std::forward<F>(f), std::make_index_sequence<sizeof...(Rest)>());
        }

    private:
        std::tuple<Valuelike<T>, Valuelike<Rest>...> m_values;

        template<typename F, typename R, std::size_t... Indices>
        Value<R> mapImpl(F&& f, std::index_sequence<Indices...> /* indices */) {
            return Value<R>{std::make_unique<DerivedValueImpl<R, T, Rest...>>(
                std::forward<F>(f),
                std::move(std::get<0>(m_values)),
                std::move(std::get<Indices + 1>(m_values))...
            )};
        }
    };

    namespace detail {
        template<typename T>
        struct DeduceValue {
            using Type = std::decay_t<T>;
        };

        template<typename T>
        struct DeduceValue<Value<T>> {
            using Type = T;
        };

        template<typename T>
        struct DeduceValue<Valuelike<T>> {
            using Type = T;
        };

        template<typename T>
        using DeduceValueType = typename DeduceValue<std::decay_t<T>>::Type;
    } // namespace detail

    template<typename T, typename... Rest>
    CombinedValues<
        detail::DeduceValueType<T>,
        detail::DeduceValueType<Rest>...
    > combine(T&& t, Rest&&... rest) {
        return {
            Valuelike<detail::DeduceValueType<T>>{std::forward<T>(t)},
            Valuelike<detail::DeduceValueType<Rest>>{std::forward<Rest>(rest)}...
        };
    }

    template<typename T>
    class Valuelike final {
    public:
        explicit Valuelike() noexcept
            : m_targetValue(nullptr)
            , m_ownValue(nullptr)
            , m_immediate(std::nullopt) {

        }

        // Valuelike is implictly constructible from a reference
        // to a matching value, in which case it will point to that value
        // and contain no fixed value.
        Valuelike(const Value<T>& target) noexcept
            : m_targetValue(target.m_impl.get())
            , m_ownValue(nullptr)
            , m_immediate(std::nullopt) {

        }

        Valuelike(Value<T>&& target) noexcept
            : m_targetValue(nullptr)
            , m_ownValue(std::move(target.m_impl))
            , m_immediate(std::nullopt) {
            
        }

        // Valuelike is implicitly constructible from anything that can be
        // used to construct a value of type T, in which case it will not point
        // to any value and will instead contain a fixed value.
        template<
            typename... Args,
            std::enable_if_t<
                (sizeof...(Args) > 0) &&
                !std::is_same_v<const Value<T>&, Args...> &&
                std::is_constructible_v<T, Args...>
            >* = nullptr
        >
        Valuelike(Args&&... args)
            : m_targetValue(nullptr)
            , m_ownValue(nullptr)
            , m_immediate(std::in_place, std::forward<Args>(args)...) {

        }

        Valuelike(Valuelike&& v) noexcept
            : m_targetValue(std::exchange(v.m_targetValue, nullptr))
            , m_ownValue(std::move(v.m_ownValue))
            , m_immediate(std::move(v.m_immediate)) {
            assert(!pointsToValue() || !isImmediate());
            assert(!pointsToValue() || (hasTargetValue() != hasOwnValue()));
        }

        ~Valuelike() noexcept = default;

        // Returns a copy of the Valuelike, except that if the original owns
        // its own derived value, the returned copy will point to that derived
        // value instead of owning a copy.
        Valuelike view() const noexcept {
            static_assert(std::is_copy_constructible_v<T>, "The stored type must be copy constructible to take a view");
            if (m_immediate.has_value()) {
                return static_cast<const T&>(*m_immediate);
            } else if (m_targetValue){
                return Valuelike{m_targetValue};
            } else if (m_ownValue) {
                return Valuelike{m_ownValue.get()};
            } else {
                return Valuelike{};
            }
        }

        Valuelike& operator=(Valuelike&& v) noexcept {
            if (&v == this) {
                return *this;
            }
            assert(!v.pointsToValue() || !v.isImmediate());
            assert(!v.pointsToValue() || (v.hasTargetValue() != v.hasOwnValue()));
            assert(!pointsToValue() || !isImmediate());
            assert(!pointsToValue() || (hasTargetValue() != hasOwnValue()));
            m_targetValue = std::exchange(v.m_targetValue, nullptr);
            m_ownValue = std::move(v.m_ownValue);
            m_immediate = std::move(v.m_immediate);
            assert(!pointsToValue() || !isImmediate());
            assert(!pointsToValue() || (hasTargetValue() != hasOwnValue()));
            return *this;
        }

        Valuelike& operator=(const Valuelike&) = delete;

        bool hasTargetValue() const noexcept {
            return m_targetValue != nullptr;
        }

        bool hasOwnValue() const noexcept {
            return m_ownValue != nullptr;
        }

        bool pointsToValue() const noexcept {
            return hasTargetValue() || hasOwnValue();
        }

        bool isImmediate() const noexcept {
            return m_immediate.has_value();
        }

        bool hasSomething() const noexcept {
            assert(!pointsToValue() || !isImmediate());
            assert(!pointsToValue() || (hasTargetValue() != hasOwnValue()));
            return pointsToValue() || isImmediate();
        }

        const T& getOnce() const & noexcept {
            assert(!pointsToValue() || !isImmediate());
            assert(!pointsToValue() || (hasTargetValue() != hasOwnValue()));
            assert(hasSomething());
            if (m_targetValue) {
                return m_targetValue->getOnce();
            } else if (m_ownValue) {
                return m_ownValue->getOnce();
            } else {
                assert(m_immediate.has_value());
                return *m_immediate;
            }
        }

        // For expiring Valuelike objects, getOnce() is overloaded to
        // return by value, avoiding a dangling reference
        T getOnce() const && noexcept {
            T ret = static_cast<const Valuelike&>(*this).getOnce();
            return ret;
        }

        void reset() {
            assert(!pointsToValue() || !isImmediate());
            assert(!pointsToValue() || (hasTargetValue() != hasOwnValue()));
            m_targetValue = nullptr;
            m_ownValue = nullptr;
            m_immediate.reset();
        }

        template<typename F>
        auto map(F&& f) const & {
            using R = std::invoke_result_t<std::decay_t<F>, DiffArgType<T>>;

            if (isImmediate()) {
                return Valuelike<R>{f(getOnce())};
            } else if (pointsToValue()){
                auto v = getValue();
                assert(v);
                return Valuelike<R>{v->map(std::forward<F>(f))};
            } else {
                return Valuelike<R>{};
            }
        }

        template<typename F>
        auto map(F&& f) && {
            return combine(std::move(*this)).map(std::forward<F>(f));
        }

    private:
        Valuelike(const ValueImpl<T>* impl)
            : m_targetValue(impl)
            , m_ownValue(nullptr)
            , m_immediate(std::nullopt) {
        
        }

        const ValueImpl<T>* getValue() const noexcept {
            assert(!pointsToValue() || !isImmediate());
            assert(!pointsToValue() || (hasTargetValue() != hasOwnValue()));
            if (m_targetValue) {
                return m_targetValue;
            }
            return m_ownValue.get();
        }

        const ValueImpl<T>* m_targetValue;
        std::unique_ptr<ValueImpl<T>> m_ownValue;
        std::optional<T> m_immediate;

        friend ValueImpl<T>;
        friend Observer<T>;
    };

    class ObserverBase;

    class ObserverOwner {
    public:
        ObserverOwner() noexcept;
        ObserverOwner(ObserverOwner&&) noexcept;
        virtual ~ObserverOwner() noexcept;

        ObserverOwner(const ObserverOwner&) = delete;
        ObserverOwner& operator=(ObserverOwner&&) = delete;
        ObserverOwner& operator=(const ObserverOwner&) = delete;

        bool isActive() const noexcept;

        void setActive(bool active) noexcept;

    private:
        std::vector<ObserverBase*> m_ownObservers;
        bool m_active;

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
        Observer(ObserverOwnerType* self, void (ObserverOwnerType::* onUpdate)(DiffArgType<T>), Valuelike<T> vl)
            : ObserverBase(self)
            , m_valuelike(std::move(vl))
            , m_onUpdate(makeUpdateFunction(self, onUpdate)) {

            if (auto vv = m_valuelike.getValue()) {
                auto& v = vv->m_observers;
                assert(std::count(v.begin(), v.end(), this) == 0);
                v.push_back(this);
            }
        }
        template<typename ObserverOwnerType>
        Observer(ObserverOwnerType* self, void (ObserverOwnerType::* onUpdate)(DiffArgType<T>))
            : ObserverBase(self)
            , m_valuelike()
            , m_onUpdate(makeUpdateFunction(self, onUpdate)) {

        }

        Observer(Observer&& o) noexcept 
            : ObserverBase(std::move(o))
            , m_valuelike(std::move(o.m_valuelike))
            , m_onUpdate(std::exchange(o.m_onUpdate, nullptr)) {
            if (auto vl = m_valuelike.getValue()) {
                auto& v = vl->m_observers;
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
            m_valuelike = std::move(o.m_valuelike);
            m_onUpdate = std::exchange(o.m_onUpdate, nullptr);
            if (auto vl = m_valuelike.getValue()) {
                auto& v = vl->m_observers;
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

        void assign(const Value<T>& target) {
            assert(!target.m_previousValue.has_value());

            const auto diff = [&] {
                if (m_valuelike.hasSomething()) {
                    return Difference<T>::compute(
                        Summary<T>::compute(m_valuelike.getOnce()),
                        target.getOnce()
                    );
                } else {
                    return Difference<T>::computeFirst(target.getOnce());
                }
            }();
            reset();
            assert(!m_valuelike.hasSomething());
            m_valuelike = target;
            assert(m_valuelike.hasTargetValue());
            assert(!m_valuelike.hasOwnValue());
            assert(!m_valuelike.isImmediate());
            update(diff);
            auto vl = m_valuelike.getValue();
            assert(vl);
            auto& v = vl->m_observers;
            assert(std::count(v.begin(), v.end(), this) == 0);
            v.push_back(this);
        }

        void assign(T fixedValue) {
            const auto diff = [&] {
                if (m_valuelike.hasSomething()) {
                    return Difference<T>::compute(
                        Summary<T>::compute(m_valuelike.getOnce()),
                        static_cast<const T&>(fixedValue)
                    );
                } else {
                    return Difference<T>::computeFirst(
                        static_cast<const T&>(fixedValue)
                    );
                }
            }();
            reset();
            assert(!m_valuelike.hasSomething());

            m_valuelike = std::move(fixedValue);
            update(diff);
            assert(m_valuelike.isImmediate());
            assert(!m_valuelike.hasTargetValue());
            assert(!m_valuelike.hasOwnValue());
        }

        void assign(Valuelike<T>&& vl) {
            assert(vl.hasSomething());
            const auto diff = [&] {
                if (m_valuelike.hasSomething()) {
                    return Difference<T>::compute(
                        Summary<T>::compute(m_valuelike.getOnce()),
                        static_cast<const T&>(vl.getOnce())
                    );
                } else {
                    return Difference<T>::computeFirst(vl.getOnce());
                }
            }();
            m_valuelike = std::move(vl);
            
            if (auto vv = m_valuelike.getValue()){
                auto& v = vv->m_observers;
                assert(std::count(v.begin(), v.end(), this) == 0);
                v.push_back(this);
            }
            update(diff);
        }

        const Valuelike<T>& getValuelike() const noexcept {
            return m_valuelike;
        }

        void reset() {
            if (auto vl = m_valuelike.getValue()) {
                auto& v = vl->m_observers;
                assert(std::count(v.begin(), v.end(), this) == 1);
                auto it = std::find(v.begin(), v.end(), this);
                assert(it != v.end());
                v.erase(it);
            }
            m_valuelike.reset();
        }

        void update(DiffArgType<T> t) {
            assert(m_onUpdate);
            assert(owner());
            m_onUpdate(owner(), t);
        }

    private:
        Valuelike<T> m_valuelike;

        std::function<void(ObserverOwner*, DiffArgType<T>)> m_onUpdate;

        template<typename ObserverOwnerType>
        static std::function<void(ObserverOwner*, DiffArgType<T>)> makeUpdateFunction(ObserverOwnerType* /* self */, void (ObserverOwnerType::* onUpdate)(DiffArgType<T>)) {
            static_assert(std::is_base_of_v<ObserverOwner, ObserverOwnerType>, "ObserverOwnerType must derive from ObserverOwner");
            // NOTE: self could be captured here, but is not, because it will become a
            // dangling pointer if the ObserverOwner is moved, which will be often in the
            // case of Components being passed stored inside AnyComponent
            return [onUpdate](ObserverOwner* self, DiffArgType<T> t){
                assert(self);
                assert(dynamic_cast<ObserverOwnerType*>(self));
                auto sd = static_cast<ObserverOwnerType*>(self);
                if (!sd->isActive()) {
                    return;
                }
                (sd->*onUpdate)(t);
            };
        }

        friend Value<T>;
    };



    namespace detail {
        template<typename Derived, std::size_t Index, typename T, typename... Rest>
        class DerivedValueBaseImpl;

        template<typename Derived, std::size_t Index, typename T, typename U, typename... Rest>
        class DerivedValueBaseImpl<Derived, Index, T, U, Rest...> : public DerivedValueBaseImpl<Derived, Index + 1, U, Rest...> {
        public:
            using Base = DerivedValueBaseImpl<Derived, Index + 1, U, Rest...>;

            DerivedValueBaseImpl(Derived* self, Valuelike<T>&& vt, Valuelike<U>&& vu, Valuelike<Rest>&&... vrest)
                : Base(self, std::move(vu), std::move(vrest)...)
                , m_observer(
                    self,
                    static_cast<void (Derived::*)(DiffArgType<T>)>(&DerivedValueBaseImpl::onUpdate),
                    std::move(vt)
                ) {

            }

            template<std::size_t I>
            decltype(auto) getOnce() noexcept {
                if constexpr (I == Index) {
                    return m_observer.getValuelike().getOnce();
                } else {
                    return Base::template getOnce<I>();
                }
            }

        private:
            Observer<T> m_observer;

            void onUpdate(DiffArgType<T> d) {
                static_cast<Derived*>(this)->updateFrom<T, Index>(d);
            }
        };

        template<typename Derived, std::size_t Index, typename T>
        class DerivedValueBaseImpl<Derived, Index, T> {
        public:

            DerivedValueBaseImpl(Derived* self, Valuelike<T>&& v)
                : m_observer(self, static_cast<void (Derived::*)(DiffArgType<T>)>(&DerivedValueBaseImpl::onUpdate), std::move(v)) {

            }

            template<std::size_t I>
            decltype(auto) getOnce() noexcept {
                static_assert(I == Index, "Something is wrong here");
                return m_observer.getValuelike().getOnce();
            }

        private:
            Observer<T> m_observer;

            void onUpdate(DiffArgType<T> d) {
                static_cast<Derived*>(this)->updateFrom<T, Index>(d);
            }
        };

    } // namespace detail

    // Value that is a pure function of one or more values
    // TODO: can this be made more efficient when multiple input
    // values have changed?
    template<typename T, typename U, typename... Rest>
    class DerivedValueImpl
        : public ValueImpl<T>
        , public ObserverOwner
        , public detail::DerivedValueBaseImpl<DerivedValueImpl<T, U, Rest...>, 0, U, Rest...> {
    public:
        using SelfType = DerivedValueImpl<T, U, Rest...>;
        using Base = detail::DerivedValueBaseImpl<SelfType, 0, U, Rest...>;

        using FunctionType = std::function<T(DiffArgType<U>, DiffArgType<Rest>...)>;

        DerivedValueImpl(FunctionType fn, Valuelike<U> u, Valuelike<Rest>... rest)
            : ValueImpl<T>(fn(
                Difference<U>::computeFirst(u.getOnce()),
                Difference<Rest>::computeFirst(rest.getOnce())...
            ))
            , Base(this, std::move(u), std::move(rest)...)
            , m_fn(std::move(fn)) {

        }


        template<typename V, std::size_t Index>
        void updateFrom(DiffArgType<V> d) {
            updateFromImpl<V, Index>(d, std::make_index_sequence<sizeof...(Rest) + 1>());
        }

    private:
        FunctionType m_fn;

        template<typename V, std::size_t Which, std::size_t... AllIndices>
        void updateFromImpl(DiffArgType<V>& d, std::index_sequence<AllIndices...> /* allIndices */) {
            assert(m_fn);
            ValueImpl<T>::set(m_fn(
                getArgument<V, Which, AllIndices>(d)...
            ));
        }

        template<typename V, std::size_t Which, std::size_t Current>
        decltype(auto) getArgument(DiffArgType<V> d) {
            if constexpr (Which == Current) {
                return d;
            } else {
                // TODO: remove this no-op once MSVC figures out that no, this parameter is not actually unreferenced
                (void)d;

                using R = std::tuple_element_t<Current, std::tuple<U, Rest...>>;
                const auto& v = Base::template getOnce<Current>();
                const auto s = Summary<R>::compute(v);
                return Difference<R>::compute(s, v);
            }
        }
    };

} // namespace ofc
