#include "catch.hpp"
#include "event.h"

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
