//          Copyright Elnar Dakeshov 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <bitset>
#include <tuple>
#include <type_traits>

namespace entityplus::meta {
/* -----------------------
** One liners
** -----------------------
*/

template <typename...>
struct typelist;

template <typename... Ts>
constexpr inline bool delay = false;

/* -----------------------
** tuple_from_typelist
** -----------------------
*/

template <typename Typelist, template <class> class Container>
struct tuple_from_typelist;

template <typename... Ts, template <class> class Container>
struct tuple_from_typelist<typelist<Ts...>, Container> {
    using type = std::tuple<Container<Ts>...>;
};

template <typename Typelist, template <class> class Container>
using tuple_from_typelist_t = typename tuple_from_typelist<Typelist, Container>::type;

/* -----------------------
** typelist_has_type
** -----------------------
*/

template <typename T, typename Typelist>
struct typelist_has_type;

template <typename T, typename... Ts>
struct typelist_has_type<T, typelist<Ts...>> : std::disjunction<std::is_same<T, Ts>...> {};

template <typename T, typename U>
constexpr inline bool typelist_has_type_v = typelist_has_type<T, U>::value;

/* -----------------------
** is_typelist_unique
** -----------------------
*/

template <typename Typelist>
struct is_typelist_unique;

template <>
struct is_typelist_unique<typelist<>> : std::true_type {};

template <typename T, typename... Ts>
struct is_typelist_unique<typelist<T, Ts...>>
    : std::conjunction<std::negation<typelist_has_type<T, typelist<Ts...>>>,
                       is_typelist_unique<typelist<Ts...>>> {};

template <typename T>
constexpr inline bool is_typelist_unique_v = is_typelist_unique<T>::value;

/* -----------------------
** typelist_index
** Prereq: Typelist must contain T
** -----------------------
*/

template <typename T, typename Typelist>
struct typelist_index;

template <typename T, typename... Ts>
struct typelist_index<T, typelist<T, Ts...>> : std::integral_constant<std::size_t, 0> {};

template <typename T, typename U, typename... Ts>
struct typelist_index<T, typelist<U, Ts...>>
    : std::integral_constant<std::size_t, 1 + typelist_index<T, typelist<Ts...>>::value> {};

template <typename T, typename U>
constexpr inline std::size_t typelist_index_v = typelist_index<T, U>::value;

/* -----------------------
** typelist_type
** Prereq: Typelist must contain T
** -----------------------
*/

template <std::size_t Idx, typename Typelist>
struct typelist_type;

template <typename T, typename... Ts>
struct typelist_type<0, typelist<T, Ts...>> {
    using type = T;
};

template <std::size_t Idx, typename T, typename... Ts>
struct typelist_type<Idx, typelist<T, Ts...>> {
    using type = typename typelist_type<Idx - 1, typelist<Ts...>>::type;
};

template <std::size_t I, typename T>
using typelist_type_t = typename typelist_type<I, T>::type;

/* -----------------------
** typelist_concat
** -----------------------
*/

template <typename T, typename U>
struct typelist_concat;

template <typename... Ts, typename... Us>
struct typelist_concat<typelist<Ts...>, typelist<Us...>> {
    using type = typelist<Ts..., Us...>;
};

template <typename T, typename U>
using typelist_concat_t = typename typelist_concat<T, U>::type;

/* -----------------------
** typelist_intersection
** -----------------------
*/

template <typename T, typename U>
struct typelist_intersection;

template <typename... Us>
struct typelist_intersection<typelist<>, typelist<Us...>> {
    using type = typelist<>;
};

template <typename T, typename... Ts, typename... Us>
struct typelist_intersection<typelist<T, Ts...>, typelist<Us...>> {
    using type = std::conditional_t<
        typelist_has_type_v<T, typelist<Us...>>,
        typelist_concat_t<typelist<T>,
                          typename typelist_intersection<typelist<Ts...>, typelist<Us...>>::type>,
        typename typelist_intersection<typelist<Ts...>, typelist<Us...>>::type>;
};

template <typename T, typename U>
using typelist_intersection_t = typename typelist_intersection<T, U>::type;

/* -----------------------
** get
** -----------------------
*/

template <typename T, typename List, typename Typelist>
decltype(auto) get(Typelist& t) {
    return std::get<typename List::template container_type<T>>(t);
}

template <typename T, typename List, typename Typelist>
decltype(auto) get(const Typelist& t) {
    return std::get<typename List::template container_type<T>>(t);
}

/* -----------------------
** type_bitset
** -----------------------
*/

template <typename Typelist>
class type_bitset;

template <typename... Ts>
class type_bitset<typelist<Ts...>> : private std::bitset<sizeof...(Ts)> {
    using underlying_type = std::bitset<sizeof...(Ts)>;

    type_bitset(underlying_type&& ut) noexcept : underlying_type(std::move(ut)) {}

public:
    type_bitset() = default;
    using underlying_type::operator[];

    constexpr type_bitset operator&(const type_bitset& other) const {
        return {static_cast<const underlying_type&>(*this) &
                static_cast<const underlying_type&>(other)};
    }

    constexpr type_bitset operator|(const type_bitset& other) const {
        return {static_cast<const underlying_type&>(*this) |
                static_cast<const underlying_type&>(other)};
    }

    constexpr bool operator==(const type_bitset& other) const {
        return static_cast<const underlying_type&>(*this) ==
               static_cast<const underlying_type&>(other);
    }

    constexpr bool operator!=(const type_bitset& other) const {
        return !(*this == other);
    }
};

template <typename T, typename... Ts>
decltype(auto) get(const type_bitset<typelist<Ts...>>& ts) {
    return ts[typelist_index_v<T, typelist<Ts...>>];
}

template <typename T, typename... Ts>
decltype(auto) get(type_bitset<typelist<Ts...>>& ts) {
    return ts[typelist_index_v<T, typelist<Ts...>>];
}

namespace detail {
    template <typename T>
    struct type_holder {
        using type = T;
    };

    template <std::size_t Offset, typename... Ts, typename Func, std::size_t... Is>
    inline void for_each_impl(const type_bitset<typelist<Ts...>>&, Func&& func,
                              std::index_sequence<Is...>) {
        ((void)func(Is + Offset, type_holder<typelist_type_t<Is + Offset, typelist<Ts...>>>{}),
         ...);
    }
} // namespace detail

template <std::size_t Offset, typename... Ts, typename Func>
inline void for_each(const type_bitset<typelist<Ts...>>& tb, Func&& func) {
    detail::for_each_impl<Offset>(tb, std::forward<Func>(func),
                                  std::make_index_sequence<sizeof...(Ts) - Offset>{});
}

namespace detail {
    template <typename T, typename U>
    struct make_key_impl;

    template <typename... Ts, typename... Us>
    struct make_key_impl<typelist<Ts...>, typelist<Us...>> {
        static auto make() {
            type_bitset<typelist<Us...>> ret;
            ((get<Ts>(ret) = true), ...);
            return ret;
        }
    };
} // namespace detail

template <typename T, typename U>
auto make_key() {
    return detail::make_key_impl<T, U>::make();
}

template <typename T>
struct type_print;
} // namespace entityplus::meta
