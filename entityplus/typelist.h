//          Copyright Elnar Dakeshov 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "container.h"
#include "metafunctions.h"

#include <cstdint>

namespace entityplus {
namespace detail {
    using entity_id_t = std::uintmax_t;
} // namespace detail

template <typename... Ts>
struct tag_list {
    static_assert(meta::is_typelist_unique_v<meta::typelist<Ts...>>, "tag_list must be unique");
};

template <typename... Ts>
struct component_list {
    static_assert(meta::is_typelist_unique_v<meta::typelist<Ts...>>,
                  "component_list must be unique");

    template <typename T>
    using container_type = flat_map<detail::entity_id_t, T>;
    using type = std::tuple<container_type<Ts>...>;
};
} // namespace entityplus
