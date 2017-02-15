#pragma once

#include "metafunctions.h"

#include <boost/container/flat_set.hpp>
#include <boost/container/flat_map.hpp>
#include <cstdint>

namespace entityplus {
namespace detail {
	using entity_id_t = std::uintmax_t;
}

template <typename... Ts>
struct tag_list {
	static_assert(meta::is_typelist_unique_v<meta::typelist<Ts...>>, "tag_list must be unique");
};

template <typename... Ts>
struct component_list {
	static_assert(meta::is_typelist_unique_v<meta::typelist<Ts...>>, "component_list must be unique");

	template <typename T>
	using container_type = boost::container::flat_map<detail::entity_id_t, T>;
	using type = std::tuple<container_type<Ts>...>;
};
}
