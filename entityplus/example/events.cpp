#include <entityplus/entity.h>
#include <iostream>
#include <string>

using namespace entityplus;

struct Name {
	std::string name;
};

template <typename Entity>
struct CombatEvent {
	Entity attacker, defender;
};

int main() {
	using components = component_list<int, Name>;
	using tags = tag_list<struct Player>;
	using entity_t = typename entity_manager<components, tags>::entity_t;

	entity_manager<components, tags> entityManager;
	event_manager<components, tags, CombatEvent<entity_t>> eventManager;

	entityManager.set_event_manager(eventManager);

	eventManager.subscribe<entity_created<entity_t>>([](const auto &) {
		std::cout << "entity_created\n";
	});	
	eventManager.subscribe<entity_destroyed<entity_t>>([](const auto &ev) {
		std::cout << "entity_destroyed with:\n";
		if (ev.entity.has_component<int>())
			std::cout << "int: " << ev.entity.get_component<int>() << "\n";
		if (ev.entity.has_component<Name>())
			std::cout << "Name: " << ev.entity.get_component<Name>().name << "\n";
		if (ev.entity.has_tag<Player>())
			std::cout << "Player tag\n";
	});
	eventManager.subscribe<component_added<entity_t, int>>([](const auto &ev) {
		std::cout << "int component added with value: " << ev.component << "\n";
	});

	for (int i = 0; i < 10; ++i) {
		entityManager.create_entity(i);
	}
	auto player = entityManager.create_entity();
	player.set_tag<Player>(true);
	player.add_component(Name{"Player"});

	auto ents = entityManager.get_entities<>();
	for (auto &ent : ents) ent.destroy();
	char c;
	std::cin >> c;
}
