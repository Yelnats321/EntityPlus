#pragma once
#include <catch.hpp>
#include "../entity.h"

using namespace entityplus;

#ifdef ENTITYPLUS_NO_EXCEPTIONS
inline void error_handler(error_code_t, const char *) {
	throw "threw";
}
#endif
