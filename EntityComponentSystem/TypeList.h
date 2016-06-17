#pragma once

namespace entityplus {
namespace detail {
	using EntityID = std::size_t;
}

template <typename... Ts>
struct TagList {
	template <typename T>
	struct containerType : public std::unordered_set<detail::EntityID> {};
	using type = std::tuple<containerType<Ts>...>;
	static_assert(meta::is_tuple_unique<std::tuple<Ts...>>::value, "TagList must be unique");
	static constexpr auto size = sizeof...(Ts);
};

template <typename... Ts>
struct ComponentList {
	template <typename T>
	using containerType = std::unordered_map<detail::EntityID, T>;
	using type = std::tuple<containerType<Ts>...>;
	static_assert(meta::is_tuple_unique<std::tuple<Ts...>>::value, "ComponentList must be unique");
	static constexpr auto size = sizeof...(Ts);
};

}