//          Copyright Elnar Dakeshov 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cassert>

#define ENTITYPLUS_CHECK(condition, error)                                                         \
    if constexpr (!(condition)) {                                                                  \
        static_assert((condition), error);                                                         \
    }
#define ENTITYPLUS_CHECK_ALSO(condition, error)                                                    \
    else if constexpr (!(condition)) {                                                             \
        static_assert((condition), error);                                                         \
    }

#ifndef ENTITYPLUS_NO_EXCEPTIONS
#include <stdexcept>
#endif

namespace entityplus {
enum class error_code { BAD_ENTITY, INVALID_COMPONENT };

#ifndef ENTITYPLUS_NO_EXCEPTIONS

struct bad_entity : std::logic_error {
    using std::logic_error::logic_error;
};

struct invalid_component : std::logic_error {
    using std::logic_error::logic_error;
};

#endif

} // namespace entityplus
