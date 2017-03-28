//          Copyright Elnar Dakeshov 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "test_common.h"

TEST_CASE("entity", "[entity]") {
	default_manager em;
#ifdef ENTITYPLUS_NO_EXCEPTIONS
	em.set_error_callback(error_handler);
#endif
	default_entity ent;
	REQUIRE(ent.get_status() == entity_status::UNINITIALIZED);
	REQUIRE(em.get_entities<>().size() == 0);
	em.for_each<>([](auto) {
		REQUIRE(false);
	});
	ent = em.create_entity();
	REQUIRE(ent.get_status() == entity_status::OK);
	em.get_entities<>();
	REQUIRE(em.get_entities<>().size() == 1);
	int count = 0;
	em.for_each<>([&](auto) {
		++count;
	});
	REQUIRE(count == 1);
	ent.destroy();
	REQUIRE(ent.get_status() == entity_status::DELETED);
	REQUIRE(em.get_entities<>().size() == 0);
}

TEST_CASE("components", "[entity]") {
	entity_manager<comps, tag_list<>> em;
#ifdef ENTITYPLUS_NO_EXCEPTIONS
	em.set_error_callback(error_handler);
#endif
	REQUIRE((em.get_entities<A>().size() == 0));
	REQUIRE((em.get_entities<A, B>().size() == 0));
	em.for_each<A>([](auto, auto) {
		REQUIRE(false);
	});
	em.for_each<A,B>([](auto, auto, auto) {
		REQUIRE(false);
	});

	auto ent = em.create_entity();
	REQUIRE(!ent.has_component<A>());
	REQUIRE(!ent.has_component<B>());
	REQUIRE(!ent.has_component<C>());

	REQUIRE_THROWS(ent.get_component<A>());
	REQUIRE_THROWS(ent.get_component<B>());
	REQUIRE_THROWS(ent.get_component<C>());

	auto added = ent.add_component<A>(3);
	REQUIRE(added.second);
	REQUIRE(added.first.x == 3);
	auto added2 = ent.add_component<A>(5);
	REQUIRE(!added2.second);
	REQUIRE(added2.first.x == 3);
	REQUIRE(ent.has_component<A>());
	REQUIRE(!ent.has_component<B>());
	REQUIRE(!ent.has_component<C>());

	ent.add_component<B>("test");
	REQUIRE(ent.get_component<B>().name == "test");

	auto &entA = ent.get_component<A>();
	REQUIRE(entA.x == 3);

	entA.x = 5;
	REQUIRE(ent.get_component<A>().x == 5);

	auto removed = ent.remove_component<A>();
	REQUIRE(removed);
	REQUIRE(!ent.has_component<A>());
	REQUIRE_THROWS(ent.get_component<A>());
	auto removed2 = ent.remove_component<A>();
	REQUIRE(!removed2);

	REQUIRE(ent.has_component<B>());
	REQUIRE(!ent.has_component<C>());
}

TEST_CASE("tags", "[entity]") {
	entity_manager<component_list<>, tags> em;
#ifdef ENTITYPLUS_NO_EXCEPTIONS
	em.set_error_callback(error_handler);
#endif
	auto ent = em.create_entity();

	REQUIRE(!ent.has_tag<TA>());
	REQUIRE(!ent.has_tag<TB>());
	REQUIRE(!ent.has_tag<TC>());

	REQUIRE(!ent.set_tag<TA>(true));
	REQUIRE(ent.set_tag<TA>(true));

	REQUIRE(ent.has_tag<TA>());
	REQUIRE(!ent.has_tag<TB>());
	REQUIRE(!ent.has_tag<TC>());

	auto entCopy = ent;

	REQUIRE(entCopy.has_tag<TA>());
	REQUIRE(!entCopy.has_tag<TB>());
	REQUIRE(!entCopy.has_tag<TC>());

	REQUIRE(ent.set_tag<TA>(false));
	REQUIRE(!ent.has_tag<TA>());
}

TEST_CASE("stale entity", "[entity]") {
	entity_manager<comps, tags> em;
#ifdef ENTITYPLUS_NO_EXCEPTIONS
	em.set_error_callback(error_handler);
#endif

	auto ent = em.create_entity();
	REQUIRE(ent.get_status() == entity_status::OK);

	auto entCopy = ent;
	ent.add_component<A>(3);
	REQUIRE(entCopy.get_status() == entity_status::STALE);
	REQUIRE_THROWS(entCopy.get_component<A>());
	REQUIRE_THROWS(entCopy.set_tag<TA>(true));

	entCopy = ent;
	REQUIRE(entCopy.get_status() == entity_status::OK);
	ent.set_tag<TA>(true);
	REQUIRE(entCopy.get_status() == entity_status::STALE);
	REQUIRE_THROWS(entCopy.set_tag<TA>(true));
}

TEST_CASE("get_entities", "[entity]") {
	SECTION("by type") {
		entity_manager<comps, tags> em;
#ifdef ENTITYPLUS_NO_EXCEPTIONS
		em.set_error_callback(error_handler);
#endif
		auto ent1 = em.create_entity();
		ent1.set_tag<TA>(true);
		ent1.set_tag<TB>(true);
		ent1.set_tag<TC>(true);
		auto ent2 = em.create_entity();
		ent2.set_tag<TA>(true);
		ent2.set_tag<TB>(true);
		auto ent3 = em.create_entity();
		ent3.set_tag<TB>(true);
		auto ent4 = em.create_entity();
		ent4.set_tag<TC>(true);
		auto ent5 = em.create_entity();

		auto all = em.get_entities<>();
		REQUIRE(all.size() == 5);
		auto makeIsIn = [](const auto & vec) {
			return [&](const auto & ent) {
				return std::find(vec.begin(), vec.end(), ent) != vec.end();
			};
		};
		{
			auto isInAll = makeIsIn(all);
			REQUIRE((isInAll(ent1) && isInAll(ent2) && isInAll(ent3) && isInAll(ent4) && isInAll(ent5)));
		}
		{
			auto vecTA = em.get_entities<TA>();
			auto isInTA = makeIsIn(vecTA);
			REQUIRE((isInTA(ent1) && isInTA(ent2)));
		}
		{
			auto vecTB = em.get_entities<TB>();
			auto isInTB = makeIsIn(vecTB);
			REQUIRE((isInTB(ent1) && isInTB(ent2) && isInTB(ent3)));
		}
		{
			auto vecTC = em.get_entities<TC>();
			auto isInTC = makeIsIn(vecTC);
			REQUIRE((isInTC(ent1) && isInTC(ent4)));
		}
	}
}


TEST_CASE("for_each entity", "[entity]") {
	entity_manager<comps, tags> em;
#ifdef ENTITYPLUS_NO_EXCEPTIONS
	em.set_error_callback(error_handler);
#endif
	auto ent1 = em.create_entity();
	ent1.add_component<A>(4);
	auto &eb = ent1.add_component<B>("smith").first;
	auto &ec = ent1.add_component<C>(3, 5).first;

	auto ent2 = em.create_entity();
	ent2.add_component<A>(2);

	em.for_each<A, B, C>([&](auto ent, auto &a, auto &b, auto &c) {
		REQUIRE(ent == ent1);
		REQUIRE(a.x == 4);
		REQUIRE(b.name == "smith");
		REQUIRE(c.get() == 5);
		REQUIRE(&eb == &b);
		REQUIRE(&ec == &c);
		b.name = "john";
	});
	REQUIRE(eb.name == "john");
	int count = 0;
	int val = 0;
	em.for_each<A>([&](auto, auto &a) {
		++count;
		val += a.x;
	});
	REQUIRE(count == 2);
	REQUIRE(val == 6);

	em.for_each<A, B, TA>([&](auto, auto &, auto &) {
		REQUIRE(false);
	});
}

TEST_CASE("for_each with control", "[entity]") {
	entity_manager<comps, tags> em;
#ifdef ENTITYPLUS_NO_EXCEPTIONS
	em.set_error_callback(error_handler);
#endif
	em.create_entity().set_tag<TA>(true);
	em.create_entity().set_tag<TA>(true);
	em.create_entity().set_tag<TA>(true);

	int count = 0;
	em.for_each<TA>([&](auto, auto& control) {
		if (++count == 1) control.breakout = true;
	});
	REQUIRE(count == 1);
	count = 0;

	em.for_each<TA>([&](auto) {
		++count;
	});
	REQUIRE(count == 3);
}



TEST_CASE("entity metafunction", "[entity]") {
	//entity_manager<int, float> em;
	entity_manager<component_list<int>, tag_list<struct tag>> em;
	auto ent = em.create_entity();
	//ent.add_component<float>(4);
	//ent.add_component<int>("wasoo");
	//ent.get_component<float>();
	//ent.has_component<float>();
	//ent.remove_component<float>();
	//ent.has_tag<int>();
	//ent.set_tag<int>(true);
	//em.get_entities<int, int>();
	//em.get_entities<float, float>();
	//em.for_each<float>([]() {});
	//em.for_each<int, int>([](auto, auto, auto) {});
	//em.for_each<>([](auto, auto, auto) {});
}

TEST_CASE("entity sync", "[entity]") {
	entity_manager<comps, tags> em;
	auto ent = em.create_entity();
	auto copyEnt = ent;
	REQUIRE(ent.get_status() == entity_status::OK);
	REQUIRE(copyEnt.get_status() == entity_status::OK);
	ent.add_component<A>(0);
	REQUIRE(ent.get_status() == entity_status::OK);
	REQUIRE(copyEnt.get_status() == entity_status::STALE);
	REQUIRE(copyEnt.sync());
	REQUIRE(ent.get_status() == entity_status::OK);
	REQUIRE(copyEnt.get_status() == entity_status::OK);

	copyEnt.set_tag<TA>(true);
	REQUIRE(ent.get_status() == entity_status::STALE);
	REQUIRE(copyEnt.get_status() == entity_status::OK);
	REQUIRE(copyEnt.sync());
	REQUIRE(ent.get_status() == entity_status::STALE);
	REQUIRE(copyEnt.get_status() == entity_status::OK);
	REQUIRE(ent.sync());
	REQUIRE(ent.get_status() == entity_status::OK);
	REQUIRE(copyEnt.get_status() == entity_status::OK);
	ent.destroy();
	REQUIRE(ent.get_status() == entity_status::DELETED);
	REQUIRE(copyEnt.get_status() == entity_status::DELETED);
	REQUIRE(!ent.sync());
	REQUIRE(!copyEnt.sync());
}

TEST_CASE("entity grouping", "[entity]") {
	entity_manager<comps, tags> em;
	entity_grouping grouping, stolenGrouping;
	REQUIRE(!grouping.is_valid());
	REQUIRE(!stolenGrouping.is_valid());
	grouping = em.create_grouping<A, TA>();
	REQUIRE(grouping.is_valid());
	REQUIRE(!stolenGrouping.is_valid());
	stolenGrouping = std::move(grouping);
	REQUIRE(!grouping.is_valid());
	REQUIRE(stolenGrouping.is_valid());
	REQUIRE(!grouping.destroy());
	REQUIRE(stolenGrouping.destroy());

	auto ent1 = em.create_entity();
	ent1.add_component<A>(0);
	ent1.set_tag<TA>(true);
	grouping = em.create_grouping<A, TA>();
	ent1.set_tag<TB>(true);

	auto ent2 = em.create_entity();
	ent2.add_component<A>(0);

	ent2.set_tag<TA>(true);
}

TEST_CASE("entity partial grouping", "[entity]") {
	entity_manager<comps, tags> em;
	em.create_grouping<A, B>();

	em.create_entity().add_component<A>(0);
	em.create_entity().add_component<A>(0);
	em.create_entity().add_component<A>(0);

	auto abent = em.create_entity();
	abent.add_component<A>(0);
	abent.add_component<B>("AB");

	REQUIRE((em.get_entities<A, B>().size() == 1));
	REQUIRE((em.get_entities<A>().size() == 4));
	REQUIRE((em.get_entities<B>().size() == 1));
	REQUIRE((em.get_entities<>().size() == 4));
}

TEST_CASE("entity partial grouping superset", "[entity]") {
	entity_manager<comps, tags> em;
	em.create_grouping<TA, TB>();

	auto ent1 = em.create_entity();
	ent1.set_tag<TA>(true);
	ent1.set_tag<TB>(true);
	ent1.set_tag<TC>(true);
	REQUIRE((em.get_entities<TA, TB, TC>().size() == 1));
	REQUIRE((em.get_entities<TA, TB>().size() == 1));
	REQUIRE((em.get_entities<TA, TC>().size() == 1));
}

TEST_CASE("entity copy/move", "[entity]") {
	entity_manager<comps, tags> em;
	auto ent1 = em.create_entity();
	REQUIRE(ent1.add_component(A{3}).second);
	REQUIRE(ent1.get_component<A>().x == 3);
	A temp{4};
	auto ent2 = em.create_entity();
	REQUIRE(ent2.add_component(temp).second);
	temp.x = 5;
	REQUIRE(ent2.get_component<A>().x == 4);
}
