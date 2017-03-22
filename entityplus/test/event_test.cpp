//          Copyright Elnar Dakeshov 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "test_common.h"
#include "../event.h"

using entityplus::event_manager;
using entityplus::subscriber_handle;

template <typename... Ts>
using empty_manager = event_manager<component_list<>, tag_list<>, Ts...>;

TEST_CASE("receiving", "[event]") {
	empty_manager<int, float> em;
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
	empty_manager<int, float> em;
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

TEST_CASE("entity event", "[event]") {
	default_manager enm;
	empty_manager<int, float> em;
	enm.set_event_manager(em);
	int ents = 0;
	auto sub = em.subscribe<entity_created<default_entity>>([&](auto) { ents++; });
	REQUIRE(sub.is_valid());
	REQUIRE(ents == 0);
	auto ent1 = enm.create_entity();
	REQUIRE(ents == 1);
	REQUIRE(sub.unsubscribe());
	auto ent2 = enm.create_entity();
	REQUIRE(ents == 1);

	em.subscribe<entity_deleted<default_entity>>([&](auto) { --ents; });
	ent1.destroy();
	REQUIRE(ents == 0);
	ent2.destroy();
	REQUIRE(ents == -1);
}


TEST_CASE("component event", "[event]") {
	entity_manager<comps, tag_list<>> entMan;
	using entity_t = entity_manager<comps, tag_list<>>::entity_t;
	event_manager<comps, tag_list<>> evtMan;
	entMan.set_event_manager(evtMan);
	int compsAdded = 0;
	evtMan.subscribe<component_added<entity_t, A>>([&](const auto &event) { compsAdded++; REQUIRE(event.component.x == 2); });
	int compsRemoved = 0;
	evtMan.subscribe<component_removed<entity_t, A>>([&](const auto &event) { compsRemoved++; REQUIRE(event.component.x == 2); });

	auto ent1 = entMan.create_entity();
	ent1.add_component<A>(2);
	REQUIRE(compsAdded == 1);
	REQUIRE(compsRemoved == 0);

	ent1.remove_component<A>();
	REQUIRE(compsAdded == 1);
	REQUIRE(compsRemoved == 1);
	ent1.remove_component<A>();
	REQUIRE(compsAdded == 1);
	REQUIRE(compsRemoved == 1);

	ent1.add_component<A>(2);
	REQUIRE(compsAdded == 2);
	REQUIRE(compsRemoved == 1);

	auto ent2 = entMan.create_entity();
	ent2.add_component<A>(2);
	REQUIRE(compsAdded == 3);
	REQUIRE(compsRemoved == 1);
	ent2.add_component<A>(4);
	REQUIRE(compsAdded == 3);
	REQUIRE(compsRemoved == 1);

	ent1.destroy();
	REQUIRE(compsAdded == 3);
	REQUIRE(compsRemoved == 2);
	ent2.destroy();
	REQUIRE(compsAdded == 3);
	REQUIRE(compsRemoved == 3);
}

TEST_CASE("tag event", "[event]") {
	entity_manager<component_list<>, tags> entMan;
	using entity_t = entity_manager<component_list<>, tags>::entity_t;
	event_manager<component_list<>, tags> evtMan;
	entMan.set_event_manager(evtMan);
	int tagsAdded = 0;
	evtMan.subscribe<tag_added<entity_t, TA>>([&](auto) { tagsAdded++; });
	int tagsRemoved = 0;
	evtMan.subscribe<tag_removed<entity_t, TA>>([&](auto) { tagsRemoved++; });

	auto ent1 = entMan.create_entity();
	ent1.set_tag<TA>(true);
	REQUIRE(tagsAdded == 1);
	REQUIRE(tagsRemoved == 0);
	ent1.set_tag<TA>(true);
	REQUIRE(tagsAdded == 1);
	REQUIRE(tagsRemoved == 0);

	ent1.set_tag<TA>(false);
	REQUIRE(tagsAdded == 1);
	REQUIRE(tagsRemoved == 1);
	ent1.set_tag<TA>(false);
	REQUIRE(tagsAdded == 1);
	REQUIRE(tagsRemoved == 1);

	ent1.set_tag<TA>(true);
	REQUIRE(tagsAdded == 2);
	REQUIRE(tagsRemoved == 1);

	auto ent2 = entMan.create_entity();
	ent2.set_tag<TA>(true);
	REQUIRE(tagsAdded == 3);
	REQUIRE(tagsRemoved == 1);

	ent1.destroy();
	REQUIRE(tagsAdded == 3);
	REQUIRE(tagsRemoved == 2);
	ent2.destroy();
	REQUIRE(tagsAdded == 3);
	REQUIRE(tagsRemoved == 3);
}
