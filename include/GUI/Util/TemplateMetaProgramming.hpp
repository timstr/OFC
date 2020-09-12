#pragma once

#include <tuple>

namespace ui::tmp {
    
    template<typename T>
    struct Tag {};

    template<typename T>
    struct TautologyImpl {
        constexpr static bool Value = true;
    };

    template<typename T>
    constexpr bool Tautology = TautologyImpl<T>::Value;



    template<typename T>
    struct ContradictionImpl {
        constexpr static bool Value = false;
    };

    template<typename T>
    constexpr bool Contradiction = ContradictionImpl<T>::Value;



    template<typename T>
    struct DontDeduceImpl {
        using Type = T;
    };

    template<typename T>
    using DontDeduce = typename DontDeduceImpl<T>::Type;



    template<typename T, template<typename...> typename To>
    struct ReapplyImpl;

    template<template<typename...> typename From, typename... Args, template<typename...> typename To>
    struct ReapplyImpl<From<Args...>, To> {
        using Type = To<Args...>;
    };

    // Given a desired (uninstantiated) variadic template and a given instantiation
    // of another variadic template, returns the desired variadic template
    // instantiated with the same parameters
    template<typename FromApplied, template<typename...> typename To>
    using Reapply = typename ReapplyImpl<FromApplied, To>::Type;


    // Given a template taking two parameters and a type, effectively binds that type
    // to the first parameter and returns a new template taking a single parameter
    // which maps to the second parameter.
    template<template<typename, typename...> typename TT, typename... U>
    struct Curry {
    
        template<typename... T>
        using Result = TT<U..., T...>;
    };



    template<typename... Ts>
    struct FilterImpl;

    template<typename ToRemove, typename... Acc>
    struct FilterImpl<ToRemove, std::tuple<Acc...>> {
        using Type = std::tuple<Acc...>;
    };

    template<typename ToRemove, typename... Acc, typename T, typename... Rest>
    struct FilterImpl<ToRemove, std::tuple<Acc...>, T, Rest...>
        : std::conditional_t<
            std::is_same_v<ToRemove, T>,
            FilterImpl<ToRemove, std::tuple<Acc...>, Rest...>,
            FilterImpl<ToRemove, std::tuple<Acc..., T>, Rest...>
        > {

    };

    // Given a type to remove and a list of types, returns a std::tuple containing
    // that list of types minus any occurences of the type to remove
    template<typename ToRemove, typename... Ts>
    using Filter = typename FilterImpl<ToRemove, std::tuple<>, Ts...>::Type;



    // Adapted from https://stackoverflow.com/a/57528226/5023438
    template<typename... Ts>
    struct RemoveDuplicatesImpl;

    template<typename T, typename... Ts>
    struct RemoveDuplicatesImpl<T, Ts...> {
        using Type = T;
    };

    template<typename... Ts, typename U, typename... Us>
    struct RemoveDuplicatesImpl<std::tuple<Ts...>, U, Us...>
        : std::conditional_t<
            (std::is_same_v<U, Ts> || ...), // Does U match anything in TS?
            RemoveDuplicatesImpl<std::tuple<Ts...>, Us...>, // if yes, recurse but don't add the type to the tuple
            RemoveDuplicatesImpl<std::tuple<Ts..., U>, Us...> // if no, recurse and add the type to the tuple
        > {

    };

    // Given a list of types, returns a std::tuple containing the same types minus any
    // duplicates. In case of duplicate types anywhere in the list, only the leftmost
    // occurence is preserved.
    template<typename... Ts>
    using RemoveDuplicates = typename RemoveDuplicatesImpl<std::tuple<>, Ts...>::Type;


    // Given a list of types, inherits from all those types separately and independently
    template<typename... Bases>
    class InheritParallel : public Bases... {

    };


    // Given an initial base class (base case base class, hence BaseBase), and a list
    // of mixin templates, creates an inheritance chain with BaseBase at the root and
    // with each Mixin deriving in order from the previous class.
    // Each mixin is expected to be a class that derives from its template parameter.
    template<typename BaseBase, template<typename> typename... Mixins>
    class InheritSerial;

    template<typename BaseBase>
    class InheritSerial<BaseBase> : public BaseBase {
    public:
        template<typename... Args>
        InheritSerial(Args&&... args)
            : BaseBase(std::forward<Args>(args)...) {
            
        }
    };

    template<typename BaseBase, template<typename> typename Mixin, template<typename> typename... Rest>
    class InheritSerial<BaseBase, Mixin, Rest...>
        : public Mixin<InheritSerial<BaseBase, Rest...>> {
    public:
        template<typename... Args>
        InheritSerial(Args&&... args)
            : Mixin<InheritSerial<BaseBase, Rest...>>(std::forward<Args>(args)...) {
            
        }
    };
     

} // namespace ui::tmp
