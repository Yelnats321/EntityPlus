#pragma once
#include <tuple>
#include <bitset>
#include <type_traits>
#include <initializer_list>

namespace entityplus {
namespace meta {
/* -----------------------
** One liners
** -----------------------
*/
template <typename... Ts>
struct typelist {};

template <typename...>
struct delay: std::false_type {};

template <typename... Ts>
constexpr bool delay_v = delay<Ts...>::value;

template <typename T>
using not_ = std::integral_constant<bool, !T::value>;

template <typename T, typename U>
using and_ = std::integral_constant<bool, T::value && U::value>;

template <typename T>
const T& as_const(T &t) {
	return t;
}

/* -----------------------
** and_all
** -----------------------
*/

template <typename... Ts>
struct and_all;

template<>
struct and_all<>: std::true_type {};

template <typename T, typename... Ts>
struct and_all<T, Ts...>: std::integral_constant<bool, T::value && and_all<Ts...>::value> {};

/* -----------------------
** typelist_has_type
** -----------------------
*/

template <typename T, typename Tuple>
struct typelist_has_type;

template <typename T>
struct typelist_has_type<T, typelist<>>: std::false_type {};

template <typename T, typename... Ts>
struct typelist_has_type<T, typelist<T, Ts...>>: std::true_type {};

template <typename T, typename U, typename... Ts>
struct typelist_has_type<T, typelist<U, Ts...>>: typelist_has_type<T, typelist<Ts...>> {};

template <typename T, typename U>
constexpr bool typelist_has_type_v = typelist_has_type<T, U>::value;

/* -----------------------
** is_tuple_unique
** -----------------------
*/

template <typename Tuple>
struct is_typelist_unique;

template <>
struct is_typelist_unique<typelist<>>: std::true_type {};

template <typename T, typename... Ts>
struct is_typelist_unique<typelist<T, Ts...>> :
	and_<
		not_<typelist_has_type<T, typelist<Ts...>>>,
		is_typelist_unique<typelist<Ts...>>
	> {};

template <typename T>
constexpr bool is_typelist_unique_v = is_typelist_unique<T>::value;

/* -----------------------
** typelist_index
** -----------------------
*/

template <typename T, typename Typelist>
struct typelist_index;

template <typename T, typename... Ts>
struct typelist_index<T, typelist<T, Ts...>>
	: std::integral_constant<std::size_t, 0> {};

template <typename T, typename U, typename... Ts>
struct typelist_index<T, typelist<U, Ts...>>
	: std::integral_constant<std::size_t, 1 + typelist_index<T, typelist<Ts...>>::value> {};

template <typename T, typename U>
constexpr std::size_t typelist_index_v = typelist_index<T, U>::value;

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
	using type = std::conditional_t<typelist_has_type_v<T, typelist<Us...>>,
		typelist_concat_t<typelist<T>, typename typelist_intersection<typelist<Ts...>, typelist<Us...>>::type>,
		typename typelist_intersection<typelist<Ts...>, typelist<Us...>>::type>;
};

template <typename T, typename U>
using typelist_intersection_t = typename typelist_intersection<T, U>::type;

/* -----------------------
** for_each
** -----------------------
*/

namespace detail {
template <typename Func, typename... Ts, std::size_t... Is>
inline void for_each_impl(std::tuple<Ts...> &tup, Func &&func, std::index_sequence<Is...>) {
	(void)tup; (void)func;
	std::initializer_list<int> _ = {((void)func(std::get<Is>(tup), Is), 0)...};
}
}

template <typename Func, typename... Ts>
inline void for_each(std::tuple<Ts...> &tup, Func&& func) {
	detail::for_each_impl(tup, std::forward<Func>(func), std::index_sequence_for<Ts...>{});
}

/* -----------------------
** eval_if
** -----------------------
*/

namespace detail {
template <std::size_t N>
struct tag: tag<N - 1> {};

template <>
struct tag <0> {};

template<typename Pred, typename Func>
struct fail_cond_t {
	using pred_type = Pred;
	Func func;
};

template <typename T, typename... Ts,
	typename Pred = typename std::decay_t<T>::pred_type,
	typename = std::enable_if_t<std::is_base_of<std::false_type, Pred>::value>>
decltype(auto) get_success(tag<1>, T&& t, Ts&&...) {
	return std::forward<T>(t);
}

template <typename T, typename... Ts>
decltype(auto) get_success(tag<0>, T&&, Ts&&... ts) {
	return get_success(tag<1>{}, std::forward<Ts>(ts)...);
}
}

template <typename Pred, typename Func>
detail::fail_cond_t<Pred, Func> fail_cond(Func&& func) {
	return{std::forward<Func>(func)};
}

template <typename Func, typename... Preds, typename... Funcs>
decltype(auto) eval_if(Func&& success, detail::fail_cond_t<Preds, Funcs>&&... fcs) {
	auto &&rt = detail::get_success(
		detail::tag<1>{},
		std::move(fcs)...,
		fail_cond<std::false_type>(std::forward<Func>(success)));
	using pred_type = typename std::decay_t<decltype(rt)>::pred_type;
	return rt.func(std::false_type{});
}

/* -----------------------
** get
** -----------------------
*/

template <typename T, typename List, typename Tuple>
decltype(auto) get(Tuple &t) {
	return std::get<typename List::template container_type<T>>(t);
}

template <typename T, typename List, typename Tuple>
decltype(auto) get(const Tuple &t) {
	return std::get<typename List::template container_type<T>>(t);
}

/* -----------------------
** type_bitset
** -----------------------
*/

template <typename Tuple>
class type_bitset;

template <typename... Ts>
class type_bitset<typelist<Ts...>>: private std::bitset<sizeof...(Ts)> {
	using underlying_type = std::bitset<sizeof...(Ts)>;
	type_bitset(underlying_type && ut): underlying_type(std::move(ut)) {}
public:
	type_bitset() = default;
	using underlying_type::operator[];

	constexpr type_bitset operator&(const type_bitset &other) const {
		return{static_cast<const underlying_type &>(*this) & static_cast<const underlying_type &>(other)};
	}

	constexpr bool operator==(const type_bitset &other) const {
		return static_cast<const underlying_type &>(*this) == static_cast<const underlying_type &>(other);
	}

	constexpr bool operator!=(const type_bitset &other) const {
		return !(*this == other);
	}
};

template <typename T, typename... Ts>
decltype(auto) get(const type_bitset<typelist<Ts...>> & ts) {
	return ts[typelist_index_v<T, typelist<Ts...>>];
}

template <typename T, typename... Ts>
decltype(auto) get(type_bitset<typelist<Ts...>> & ts) {
	return ts[typelist_index_v<T, typelist<Ts...>>];
}

template <typename T>
struct type_print;
}//namespace meta
}//namespace entityplus