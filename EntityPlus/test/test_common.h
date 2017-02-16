//          Copyright Elnar Dakeshov 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <catch.hpp>
#include "../entity.h"

using namespace entityplus;

#ifdef ENTITYPLUS_NO_EXCEPTIONS
inline void error_handler(error_code_t, const char *) {
	throw "threw";
}
#endif
