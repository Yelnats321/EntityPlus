#pragma once
#include <tuple>
#include <bitset>

namespace entityplus {
namespace meta {
/* -----------------------
** One liners
** -----------------------
*/

template <typename...>
struct delay {
	static constexpr auto value = false;
};

template <typename T>
using type_not = std::integral_constant<bool, !T::value>;

template <typename T, typename U>
using type_and = std::integral_constant<bool, T::value && U::value>;

/* -----------------------
** tuple_concat
** -----------------------
*/
/*template <typename TupleA, typename TupleB>
struct tuple_concat;

template <typename... Ts, typename... Us>
struct tuple_concat<std::tuple<Ts...>, std::tuple<Us...>> {
using type = std::tuple<Ts..., Us...>;
};*/

/* -----------------------
** tuple_has_type
** -----------------------
*/
template <typename T, typename Tuple>
struct tuple_has_type;

template <typename T>
struct tuple_has_type<T, std::tuple<>> : std::false_type {};

template <typename T, typename... Ts>
struct tuple_has_type<T, std::tuple<T, Ts...>> : std::true_type {};

template <typename T, typename U, typename... Ts>
struct tuple_has_type<T, std::tuple<U, Ts...>> : tuple_has_type<T, std::tuple<Ts...>> {};

/* -----------------------
** is_tuple_unique
** -----------------------
*/
template <typename Tuple>
struct is_tuple_unique;

template <>
struct is_tuple_unique<std::tuple<>> : std::true_type {};

template <typename T, typename... Ts>
struct is_tuple_unique<std::tuple<T, Ts...>> :
	type_and<
		type_not<tuple_has_type<T, std::tuple<Ts...>> >,
		is_tuple_unique<std::tuple<Ts...>>
	> {};

/* -----------------------
** tuple_index
** -----------------------
*/
template <typename T, typename Tuple>
struct tuple_index;

template <typename T, typename... Ts>
struct tuple_index<T, std::tuple<T, Ts...>> {
	static constexpr auto value = 0;
};

template <typename T, typename U, typename... Ts>
struct tuple_index<T, std::tuple<U, Ts...>> {
	static constexpr auto value = 1 + tuple_index<T, std::tuple<Ts...>>::value;
};

/* -----------------------
** type_bitset
** -----------------------
*/
template <typename... Ts>
struct type_bitset : private std::bitset<sizeof...(Ts)> {
	template <typename T>
	decltype(auto) at() const {
		return operator[tuple_index<T, std::tuple<Ts...>>::value];
	}

	template <typename T>
	decltype(auto) at() {
		return operator[tuple_index<T, std::tuple<Ts...>>::value];
	}
};
}//namespace meta
}//namespace entityplus