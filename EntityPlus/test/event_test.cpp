//          Copyright Elnar Dakeshov 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <catch.hpp>
#include "../event.h"

using entityplus::event_manager;

TEST_CASE("receiving", "[event]") {
	event_manager<int, float> em;
	bool wasCalled = false, wasCalled2 = false;
	em.register_handler<int>([&](const int &x) {
		REQUIRE(x == 4);
		wasCalled = true;
	});
	em.register_handler<float>([&](float) {
		wasCalled = true;
	});
	int x = 4;
	em.broadcast(x);
	REQUIRE(wasCalled);
	REQUIRE(!wasCalled2);
}
