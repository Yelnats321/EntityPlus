#include "metafunctions.h"
#include "entity.h"
#include "event.h"
#include <iostream>
#include <random>
#include <chrono>

namespace ep = entityplus;
namespace epm = entityplus::meta;
//using type = entityplus::ComponentList<int, int>::type;
static_assert(epm::typelist_has_type<
	int,
			  epm::typelist<float, double, int>
>{}, "");

static_assert(epm::typelist_has_type_v<
	int,
			  epm::typelist<float, double>
> == false, "");

static_assert(epm::is_typelist_unique_v<
	epm::typelist<float, double, int>
> == true, "");
static_assert(epm::is_typelist_unique_v<
	epm::typelist<float, double, float>
> == false, "");
static_assert(epm::is_typelist_unique<
	epm::typelist<int, float, double, float>
>{} == false, "");

int main() {
	/*using MyComponentList = ep::component_list<int, float>;
	using MyTagList = ep::tag_list<struct tag>;
	ep::entity_manager<MyComponentList, MyTagList> em;

	auto ent1 = em.create_entity(), ent2 = em.create_entity();
	ent2.add_component<int>();
	ent1.add_component<float>();
	ent1.has_component<float>();
	auto & loc = ent1.get_component<float>() = 4;
	auto & loc2 = ent1.get_component<float>();
	ent1.remove_component<float>();
	std::cout << loc << " " << loc2;
	ent2.set_tag<tag>(true);
	ent2.has_tag<tag>();
	const auto & ent2c = ent2;
	ent2c.get_component<int>();
	auto ge = em.get_entities<int>();
	std::cout << ge.size() << "\n";
	em.delete_entity(ent1);
	/*ep::event_manager<int, float> ev;
	ev.register_handler<int>([](int x) {
		std::cout << x << "\n";
	});
	ev.register_handler<int>([](int x) {
		std::cout << (x+2) << "\n";
	});
	ev.register_handler<float>([](float x) {
		std::cout << x << "\n";
	});
	ev.register_handler<float>([](float x) {
		std::cout << (x + 2) << "\n";
	});
	ev.broadcast(4);
	ev.broadcast(2.3f);*/

	std::random_device rd;
	std::mt19937 rand(rd());
	std::normal_distribution<> dist;
	std::uniform_real_distribution<> uni(0, 1);

	struct accel {
		double x, y;
		accel(double x, double y):x(x), y(y) {}
	};
	struct pos {
		double x = 0, y = 0;
	};
	ep::entity_manager<ep::component_list<accel, pos>, ep::tag_list<>> emt;
	auto then = std::chrono::high_resolution_clock::now();
	for (auto i = 0u; i < 10; ++i) {
		auto ent = emt.create_entity();
		//if ((i % 3) != 0) continue;
		ent.add_component<accel>(i, i);
		ent.add_component<pos>();
	}
	auto now = std::chrono::high_resolution_clock::now();
	std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(now - then).count() << "\n";
	then = now;
	for (auto i = 0u; i < 10; ++i) {
		emt.for_each<accel, pos>([](auto, auto &ac, auto &po) {
			po.x += ac.x;
			po.y += ac.y;
		});
	}
	now = std::chrono::high_resolution_clock::now();
	std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(now - then).count() << "\n";
	then = now;

	emt.for_each<pos>([](auto, auto &po) {
		std::cout << po.x << " " << po.y << '\n';
	});
	now = std::chrono::high_resolution_clock::now();
	std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(now - then).count() << "\n";
	/*struct ent {
		accel ac;
		pos po;
		ent(double x, double y):ac{x, y} {}
	};
	std::vector<ent> ents;

	auto then = std::chrono::high_resolution_clock::now();
	for (auto i = 0u; i < 10'000; ++i) {
		ents.emplace_back(dist(rand), dist(rand));
	}
	auto now = std::chrono::high_resolution_clock::now();
	std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(now - then).count() << "\n";
	then = now;
	for (auto i = 0u; i < 10'000; ++i) {
		for (auto &ent : ents) {
			ent.po.x += ent.ac.x;
			ent.po.y += ent.ac.y;
		}
	}
	now = std::chrono::high_resolution_clock::now();
	std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(now - then).count() << "\n";
	then = now;
	auto fx = 0.0, fy = 0.0;
	for (auto &ent : ents) {
		fx += ent.ac.x;
		fy += ent.ac.y;
	}
	std::cout << fx << " " << fy << "\n";
	now = std::chrono::high_resolution_clock::now();
	std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(now - then).count() << "\n";*/

	std::cin.ignore();
}