//          Copyright Elnar Dakeshov 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <catch.hpp>
#include "../event.h"

using entityplus::event_manager;
using entityplus::subscriber_handle;

TEST_CASE("receiving", "[event]") {
	event_manager<int, float> em;
	bool wasCalled = false, wasCalled2 = false;
	auto sub1 = em.subscribe<int>([&](const int &x) {
		REQUIRE(x == 4);
		wasCalled = true;
	});
	REQUIRE(sub1.is_valid());
	auto sub2 = em.subscribe<float>([&](float) {
		wasCalled = true;
	});
	REQUIRE(sub2.is_valid());
	int x = 4;
	em.broadcast(x);
	REQUIRE(wasCalled);
	REQUIRE(!wasCalled2);

	wasCalled = false;
	sub1.unsubscribe();
	REQUIRE(!sub1.is_valid());
	em.broadcast(x);
	REQUIRE(!wasCalled);
}

TEST_CASE("event handler", "[event]") {
	event_manager<int, float> em;
	subscriber_handle<int> intEvent;
	subscriber_handle<float> floatEvent;
	REQUIRE(!intEvent.is_valid());
	REQUIRE(!floatEvent.is_valid());
	REQUIRE(!intEvent.unsubscribe());
	REQUIRE(!floatEvent.unsubscribe());
	int called = 0;
	auto tempEvent = em.subscribe<int>([&](int) {called++; });
	REQUIRE(tempEvent.is_valid());
	intEvent = std::move(tempEvent);
	REQUIRE(intEvent.is_valid());
	REQUIRE(!tempEvent.is_valid());
	REQUIRE(!tempEvent.unsubscribe());
	em.broadcast(4);
	REQUIRE(called == 1);
	REQUIRE(intEvent.unsubscribe());
	em.broadcast(3);
	REQUIRE(called == 1);
	REQUIRE(!intEvent.is_valid());
	REQUIRE(!intEvent.unsubscribe());
}
