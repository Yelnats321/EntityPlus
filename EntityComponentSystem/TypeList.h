#pragma once
#include <boost/container/flat_set.hpp>
#include <boost/container/flat_map.hpp>

namespace entityplus {
namespace detail {
	using EntityID = std::uintmax_t;
	using EntityVersion = std::uintmax_t;
}

template <typename... Ts>
struct TagList {
	static_assert(meta::is_tuple_unique<std::tuple<Ts...>>::value, "TagList must be unique");

	template <typename T>
	struct ContainerType : public boost::container::flat_set<detail::EntityID> {};
	using type = std::tuple<ContainerType<Ts>...>;

	static constexpr auto size = sizeof...(Ts);
	template <typename T>
	static ContainerType<T> get(type &t) {
		return std::get<ContainerType<T>>(t);
	}
};

template <typename... Ts>
struct ComponentList {
	static_assert(meta::is_tuple_unique<std::tuple<Ts...>>::value, "ComponentList must be unique");

	template <typename T>
	using ContainerType = boost::container::flat_map<detail::EntityID, T>;
	using type = std::tuple<ContainerType<Ts>...>;

	static constexpr auto size = sizeof...(Ts);	
	template <typename T>
	static ContainerType<T> get(type &t) {
		return std::get<ContainerType<T>>(t);
	}
};

}