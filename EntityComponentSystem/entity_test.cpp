#include <catch.hpp>
#include "entity.h"

using namespace entityplus;

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

TEST_CASE("entity", "[entity]") {
	entity_manager<component_list<>, tag_list<>> em;
	auto ent = em.create_entity();
	REQUIRE(ent.get_status() == entity_status::OK);
	REQUIRE(em.get_entities<>().size() == 1);
	em.delete_entity(ent);
	REQUIRE(ent.get_status() == entity_status::NOT_FOUND);
	REQUIRE(em.get_entities<>().size() == 0);

	entity_manager<component_list<>, tag_list<>> em2;
	auto foreign = em2.create_entity();
	REQUIRE_THROWS(em.delete_entity(foreign));
}

TEST_CASE("components", "[entity]") {
	using comps = component_list<A, B, C>;
	entity_manager<comps, tag_list<>> em;
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
	using tags = tag_list<struct TA, struct TB, struct TC>;
	entity_manager<component_list<>, tags> em;
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
	using comps = component_list<A, B, C>;
	using tags = tag_list<struct TA, struct TB, struct TC>;
	entity_manager<comps, tags> em;

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
	using comps = component_list<A, B, C>;
	using tags = tag_list<struct TA, struct TB, struct TC>;
	SECTION("by type") {
		entity_manager<comps, tags> em;
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
	using comps = component_list<A, B, C>;
	using tags = tag_list<struct TA, struct TB, struct TC>;

	entity_manager<comps, tags> em;
	auto ent1 = em.create_entity();
	auto &ea = ent1.add_component<A>(4).first;
	auto &eb = ent1.add_component<B>("smith").first;
	auto &ec = ent1.add_component<C>(3, 5).first;

	auto ent2 = em.create_entity();
	ent2.add_component<A>(2);

	em.for_each<A, B, C>([&](auto ent, auto &a, auto &b, auto &c) {
		REQUIRE(ent == ent1);
		REQUIRE(a.x == 4);
		REQUIRE(b.name == "smith");
		REQUIRE(c.get() == 5);
		//REQUIRE(&ea == &a); not true because ent2 can invalidate the component handle
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

	em.for_each<A, B, TA>([&](auto ent, auto &, auto &) {
		REQUIRE(false);
	});
}
