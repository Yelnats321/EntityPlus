//          Copyright Elnar Dakeshov 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <catch.hpp>
#include <entityplus/entity.h>

using namespace entityplus;

#ifdef ENTITYPLUS_NO_EXCEPTIONS
inline void error_handler(error_code_t, const char *) {
	throw "threw";
}
#endif

struct A {
	int x;
	A(int x): x(x) {}
};

struct B {
	std::string name;
	B(std::string name):name(name) {}
};

class C {
	int x, y;
public:
	C(int x, int y): x(x), y(y) {};
	int get() {
		return std::max(x, y);
	}
};

using default_manager = entity_manager<component_list<>, tag_list<>>;
using default_entity = typename default_manager::entity_t;

using comps = component_list<A, B, C>;
using tags = tag_list<struct TA, struct TB, struct TC>;
