#pragma once

#include <cassert>
#include <functional>
#include <memory>
#include <optional>
#include <type_traits>
#include <variant>
#include <vector>

namespace ofc {

    template<typename T>
    class Value;

    template<typename T>
    class Observer;

    class ObserverOwner;

    namespace detail {
        
        void enqueueValueUpdater(std::function<void()>);

        void updateAllValues();



        template<typename T>
        struct IsVectorImpl : std::false_type {};

        template<typename T>
        struct IsVectorImpl<std::vector<T>> : std::true_type {};

        template<typename T>
        constexpr bool IsVector = IsVectorImpl<T>::value;



        template<typename T>
        struct IsValueImpl : std::false_type {};

        template<typename T>
        struct IsValueImpl<Value<T>> : std::true_type {};

        template<typename T>
        constexpr bool IsValue = IsValueImpl<T>::value;

    } // namespace detail

    struct DefaultConstruct {};

    inline constexpr auto defaultConstruct = DefaultConstruct{};

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

        const std::vector<SummaryType<T>>& oldValue() const noexcept {
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
    class ValueImpl : public std::enable_shared_from_this<ValueImpl<T>> {
    public:
        explicit ValueImpl() noexcept
            : m_value{}
            , m_previousValue(std::nullopt) {
        
        }

        template<
            typename... Args,
            std::enable_if_t<
                (sizeof...(Args) > 0)
                && std::is_constructible_v<T, Args...>
            >* = nullptr
        >
        ValueImpl(Args&&... args)
            : m_value(std::forward<Args>(args)...)
            , m_previousValue(std::nullopt) {
        
            static_assert(std::is_constructible_v<T, Args...>);
        }

        /* ValueImpl(const T& t)
            : m_value(t)
            , m_previousValue(std::nullopt) {

        }
        ValueImpl(T&& t) noexcept
            : m_value(std::move(t))
            , m_previousValue(std::nullopt) {

        } */

        ValueImpl(const ValueImpl&) = delete;
        ValueImpl(ValueImpl&&) = delete;

        virtual ~ValueImpl() noexcept {
            for (const auto& o : m_observers) {
                assert(o);
                o->reset();
            }
        }


        ValueImpl& operator=(const ValueImpl&) = delete;
        ValueImpl& operator=(ValueImpl&&) = delete;

        const T& getOnce() const noexcept {
            return m_value;
        }

        void set(const T& t) {
            makeDirty();
            m_value = t;
        }
        void set(T&& t) {
            makeDirty();
            m_value = std::move(t);
        }

        T& getOnceMut() noexcept {
            makeDirty();
            return m_value;
        }

        template<typename F>
        auto map(F&& f) {
            using R = std::invoke_result_t<F, DiffArgType<T>>;
            static_assert(!std::is_void_v<R>);
            static_assert(std::is_base_of_v<ValueImpl<R>, DerivedValueImpl<R, T>>);
            auto shared_this = this->shared_from_this();
            assert(shared_this);
            return Value<R>{std::make_shared<DerivedValueImpl<R, T>>(
                std::forward<F>(f),
                Value<T>(std::move(shared_this))
            )};
        }

        template<typename F, typename U = T, std::enable_if_t<detail::IsVector<U>>* = nullptr>
        auto vectorMap(F&& f) {
            using ElementType = typename T::value_type;
            static_assert(std::is_invocable_v<F, CRefOrValue<ElementType>>);
            using R = std::invoke_result_t<F, CRefOrValue<ElementType>>;
            static_assert(!std::is_void_v<R>);
            static_assert(!std::is_reference_v<R>);
            auto shared_this = this->shared_from_this();
            assert(shared_this);
            return Value<std::vector<R>>{std::make_shared<VectorMappedValueImpl<R, ElementType>>(
                std::forward<F>(f),
                Value<T>{std::move(shared_this)}
            )};
        }

        // Suppose:
        //   struct X {
        //       Value<String> name;
        //       Value<int> count;
        //   };
        //   Value<std::vector<X>> vectorVal;
        //   
        //   auto sum = vectorVal.reduce(
        //       0,
        //       [](const X& x){ return x.count; } // -> Value<int>, without explicitly specifying return type as otherwise
        //       [](int acc, int c){ return acc + c; } // Unhh so simple
        //   );
        template<typename R, typename F, typename G, typename U = T, std::enable_if_t<detail::IsVector<U>>* = nullptr>
        auto reduce(R&& init, F&& elementToValue, G&& combine) {
            using ElementType = typename T::value_type;
            static_assert(std::is_invocable_v<F, CRefOrValue<ElementType>>);
            using V = std::invoke_result_t<F, CRefOrValue<ElementType>>;
            using I = typename V::Type;
            static_assert(std::is_invocable_v<G, R, CRefOrValue<I>>);
            using RR = std::invoke_result_t<G, R, CRefOrValue<I>>;
            static_assert(std::is_same_v<RR, R>); // TODO: too restrictive? use is_convertible_v instead?
            auto shared_this = this->shared_from_this();
            assert(shared_this);
            return Value<R>{std::make_shared<ReducedValueImpl<R, ElementType, I>>(
                std::forward<R>(init),
                std::forward<F>(elementToValue),
                std::forward<G>(combine),
                Value<std::vector<ElementType>>{std::move(shared_this)}
            )};
        }

        template<typename P, std::enable_if_t<std::is_member_object_pointer_v<P>>* = nullptr>
        auto project(P memptr) {
            static_assert(std::is_same_v<T, std::decay_t<DiffArgType<T>>>);
            return this->map([memptr](CRefOrValue<T> t){ return t.*memptr; });
        }

    private:
        // TODO: consider renaming to "state" and "previousState" to avoid ambiguity
        T m_value;
        std::optional<SummaryType<T>> m_previousValue;
        mutable std::vector<Observer<T>*> m_observers;

        SummaryType<T> summarize() const {
            auto result = Summary<T>::compute(static_cast<const T&>(m_value));
            static_assert(std::is_same_v<SummaryType<T>, decltype(result)>);
            return result;
        }

        void makeDirty() {
            if (!m_previousValue.has_value()) {
                m_previousValue.emplace(summarize());
                registerForUpdate();
            }
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
        using Type = T;

        explicit Value() noexcept
            : m_impl(nullptr) {
        
        }

        Value(DefaultConstruct)
            : m_impl(std::make_shared<ValueImpl<T>>()) {
        
        }

        template<
            typename... Args,
            std::enable_if_t<
                (sizeof...(Args) > 0)
                && std::is_constructible_v<T, Args...>
            >* = nullptr
        > Value(Args&&... args)
            : m_impl(std::make_shared<ValueImpl<T>>(std::forward<Args>(args)...)) {
        
        }

        /* Value(const T& t)
            : m_impl(std::make_shared<ValueImpl<T>>(t)) {

        }
        Value(T&& t)
            : m_impl(std::make_shared<ValueImpl<T>>(std::move(t))) {

        } */

        Value(Value&& v) noexcept
            : m_impl(std::move(v.m_impl)) {
        
        }
        Value(const Value& v) noexcept
            : m_impl(v.m_impl) {
        
        }

        Value& operator=(Value&& v) noexcept {
            if (&v != this) {
                m_impl = std::move(v.m_impl);
            }
            return *this;
        }
        Value& operator=(const Value& v) noexcept {
            if (&v != this) {
                m_impl = v.m_impl;
            }
            return *this;
        }

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

        void reset() {
            m_impl = nullptr;
        }

        bool hasValue() const noexcept {
            return static_cast<bool>(m_impl);
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
        
        template<typename F, typename U = T, std::enable_if_t<detail::IsVector<U>>* = nullptr>
        auto vectorMap(F&& f) const {
            assert(m_impl);
            return m_impl->vectorMap(std::forward<F>(f));
        }

        template<typename R, typename F, typename G, typename U = T, std::enable_if_t<detail::IsVector<U>>* = nullptr>
        auto reduce(R&& init, F&& elementToValue, G&& combine) {
            assert(m_impl);
            return m_impl->reduce<R>(
                std::forward<R>(init),
                std::forward<F>(elementToValue),
                std::forward<G>(combine)
            );
        }

        template<typename P, std::enable_if_t<std::is_member_object_pointer_v<P>>* = nullptr>
        auto project(P memptr) const {
            assert(m_impl);
            return m_impl->project(memptr);
        }

    private:
        Value(std::shared_ptr<ValueImpl<T>> impl) noexcept
            : m_impl(std::move(impl)) {
        
        }

        std::shared_ptr<ValueImpl<T>> m_impl;
        
        ValueImpl<T>* impl() noexcept {
            return m_impl.get();
        }

        const ValueImpl<T>* impl() const noexcept {
            return m_impl.get();
        }

        const void* summarize() const noexcept {
            return static_cast<const void*>(m_impl.get());
        }

        template<typename TT>
        friend class ValueImpl;

        friend class Observer<T>;

        template<typename TT, typename... Rest>
        friend class CombinedValues;

        friend Summary<Value<T>>;
    };

    template<typename T, typename... Rest>
    class CombinedValues final {
    public:
        CombinedValues(Value<T> t, Value<Rest>... rest) 
            : m_values(std::move(t), std::move(rest)...) {

        }

        template<typename F>
        auto map(F&& f) && noexcept {
            static_assert(std::is_invocable_v<F, DiffArgType<T>, DiffArgType<Rest>...>);
            using R = std::invoke_result_t<std::decay_t<F>, DiffArgType<T>, DiffArgType<Rest>...>;
            static_assert(!std::is_void_v<R>);
            return mapImpl<F, R>(std::forward<F>(f), std::make_index_sequence<sizeof...(Rest)>());
        }

    private:
        std::tuple<Value<T>, Value<Rest>...> m_values;

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
        using DeduceValueType = typename DeduceValue<std::decay_t<T>>::Type;
    } // namespace detail

    template<typename T, typename... Rest>
    CombinedValues<
        detail::DeduceValueType<T>,
        detail::DeduceValueType<Rest>...
    > combine(T&& t, Rest&&... rest) {
        return {
            std::forward<T>(t),
            std::forward<Rest>(rest)...
        };
    }

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
        Observer(ObserverOwnerType* self, void (ObserverOwnerType::* onUpdate)(DiffArgType<T>), Value<T> vl)
            : ObserverBase(self)
            , m_value(std::move(vl))
            , m_onUpdate(makeUpdateFunction(self, onUpdate)) {

            // TODO: refactor to prevent duplicating this and similar code
            if (auto vi = m_value.impl()) {
                vi->purgeUpdates();
                auto& v = vi->m_observers;
                assert(std::count(v.begin(), v.end(), this) == 0);
                v.push_back(this);
            }
        }

        template<typename ObserverOwnerType>
        Observer(ObserverOwnerType* self, void (ObserverOwnerType::* onUpdate)(DiffArgType<T>))
            : ObserverBase(self)
            , m_value()
            , m_onUpdate(makeUpdateFunction(self, onUpdate)) {

            if (auto vi = m_value.impl()) {
                vi->purgeUpdates();
                auto& v = vi->m_observers;
                assert(std::count(v.begin(), v.end(), this) == 0);
                v.push_back(this);
            }
        }

        Observer(Observer&& o) noexcept 
            : ObserverBase(std::move(o))
            , m_value(std::move(o.m_value))
            , m_onUpdate(std::exchange(o.m_onUpdate, nullptr)) {
            if (auto vi = m_value.impl()) {
                auto& v = vi->m_observers;
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
            m_value = std::move(o.m_value);
            m_onUpdate = std::exchange(o.m_onUpdate, nullptr);
            if (auto vi = m_value.impl()) {
                auto& v = vi->m_observers;
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

        void assign(Value<T> target) {
            // impl is either null, or impl has no previous value
            assert(!target.impl() || !target.impl()->m_previousValue.has_value());

            const auto diff = [&] {
                if (m_value.hasValue()) {
                    return Difference<T>::compute(
                        Summary<T>::compute(m_value.getOnce()),
                        target.getOnce()
                    );
                } else {
                    return Difference<T>::computeFirst(target.getOnce());
                }
            }();
            reset();
            assert(!m_value.hasValue());
            m_value = std::move(target);
            update(diff);
            if (auto vi = m_value.impl()) {
                auto& v = vi->m_observers;
                assert(std::count(v.begin(), v.end(), this) == 0);
                v.push_back(this);
            }
        }

        const Value<T>& getValue() const noexcept {
            return m_value;
        }

        void reset() {
            if (auto vi = m_value.impl()) {
                auto& v = vi->m_observers;
                assert(std::count(v.begin(), v.end(), this) == 1);
                auto it = std::find(v.begin(), v.end(), this);
                assert(it != v.end());
                v.erase(it);
            }
            m_value.reset();
        }

        void update(DiffArgType<T> t) {
            assert(m_onUpdate);
            assert(owner());
            m_onUpdate(owner(), t);
        }

    private:
        Value<T> m_value;

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

            DerivedValueBaseImpl(Derived* self, Value<T> vt, Value<U> vu, Value<Rest>... vrest)
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
                    return m_observer.getValue().getOnce();
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

            DerivedValueBaseImpl(Derived* self, Value<T> v)
                : m_observer(self, static_cast<void (Derived::*)(DiffArgType<T>)>(&DerivedValueBaseImpl::onUpdate), std::move(v)) {

            }

            template<std::size_t I>
            decltype(auto) getOnce() noexcept {
                static_assert(I == Index, "Something is wrong here");
                return m_observer.getValue().getOnce();
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

        DerivedValueImpl(FunctionType fn, Value<U> u, Value<Rest>... rest)
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


    // T : target element type
    // U : source element type
    template<typename T, typename U>
    class VectorMappedValueImpl : public ValueImpl<std::vector<T>>, public ObserverOwner {
    public:
        VectorMappedValueImpl(std::function<T(CRefOrValue<U>)> fn, Value<std::vector<U>> vl)
            : ValueImpl<std::vector<T>>(initialValue(fn, vl.getOnce()))
            , m_fn(std::move(fn))
            , m_observer(this, &VectorMappedValueImpl::updateValues, std::move(vl)) {

            assert(this->getOnce().size() == m_observer.getValue().getOnce().size());
        }

    private:
        std::function<T(CRefOrValue<U>)> m_fn;
        Observer<std::vector<U>> m_observer;

        void updateValues(const ListOfEdits<U>& loe) {
            assert(this->getOnce().size() == loe.oldValue().size());
            assert(m_fn);
            auto& v = this->getOnceMut();
            auto it = begin(v);
            for (const auto& e : loe.getEdits()) {
                if (e.insertion()) {
                    it = v.insert(it, m_fn(e.value()));
                    ++it;
                } else if (e.deletion()) {
                    assert(it != end(v));
                    it = v.erase(it);
                } else {
                    assert(e.nothing());
                    assert(it != end(v));
                    ++it;
                }
            }
            assert(this->getOnce().size() == loe.newValue().size());
        }

        static std::vector<T> initialValue(const std::function<T(CRefOrValue<U>)>& fn, const std::vector<U>& v) {
            assert(fn);
            auto out = std::vector<T>{};
            out.reserve(v.size());
            for (const auto& x : v) {
                out.push_back(fn(x));
            }
            return out;
        }
    };

    // T: target (singular)
    // U: source vector element type
    // V: intermediate type to which vector elements are mapped via Value<V>
    template<typename T, typename U, typename V>
    class ReducedValueImpl : public ValueImpl<T>, public ObserverOwner {
    public:
        using ElementToValueFn = std::function<Value<V>(CRefOrValue<U>)>;
        using CombineFn = std::function<T(T, CRefOrValue<V>)>;

        ReducedValueImpl(T init, ElementToValueFn elementToValue, CombineFn combine, Value<std::vector<U>> vl)
            : ValueImpl<T>(recompute(init, vl.getOnce(), elementToValue, combine))
            , m_init(std::move(init))
            , m_elementToValue(std::move(elementToValue))
            , m_combine(std::move(combine))
            , m_vectorObserver(this, &ReducedValueImpl::onUpdateVector, std::move(vl)) {
        
            assert(m_elementToValue);
            assert(m_combine);

            const auto& vals = m_vectorObserver.getValue().getOnce();
            m_elementObservers.reserve(vals.size());
            for (const U& u : vals) {
                Value<V> vv = m_elementToValue(u);
                auto o = Observer<V>(this, &ReducedValueImpl::onUpdateElement, std::move(vv));
                m_elementObservers.push_back(std::move(o));
            }
        }

    private:
        T m_init;
        ElementToValueFn m_elementToValue;
        CombineFn m_combine;

        void onUpdateVector(const ListOfEdits<U>& loe) {
            assert(m_elementObservers.size() == loe.oldValue().size());
            assert(m_elementToValue);
            auto it = begin(m_elementObservers);
            for (const auto& e : loe.getEdits()) {
                if (e.insertion()) {
                    const U& u = e.value();
                    Value<V> vv = m_elementToValue(u);
                    auto o = Observer<V>(this, &ReducedValueImpl::onUpdateElement, std::move(vv));
                    it = m_elementObservers.insert(it, std::move(o));
                    ++it;
                } else if (e.deletion()) {
                    assert(it != end(m_elementObservers));
                    it = m_elementObservers.erase(it);
                } else {
                    assert(e.nothing());
                    assert(it != end(m_elementObservers));
                    ++it;
                }
            }
            assert(m_elementObservers.size() == loe.newValue().size());
            fullUpdate();
        }

        void onUpdateElement(DiffArgType<V> /* unused */) {
            fullUpdate();
        }

        static T recompute(const T& init, const std::vector<U>& v, const ElementToValueFn& etv, const CombineFn& c) {
            assert(etv);
            assert(c);
            T acc = init;
            for (const U& e : v) {
                const Value<V>& val = etv(e);
                acc = c(std::move(acc), val.getOnce());
            }
            return acc;
        }

        void fullUpdate() {
            this->set(recompute(m_init, m_vectorObserver.getValue().getOnce(), m_elementToValue, m_combine));
        }

        Observer<std::vector<U>> m_vectorObserver;
        std::vector<Observer<V>> m_elementObservers;
    };

} // namespace ofc
