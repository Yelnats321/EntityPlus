#include <entityplus/entity.h>
#include <iostream>

using namespace entityplus;

int main() {
	using entity_manager_t = entity_manager<component_list<int>, tag_list<>>;
	using entity_t = entity_manager_t::entity_t;

	entity_manager_t entityManager;
	for (int i = 0; i < 50; ++i) {
		entityManager.create_entity(i);
	}

	auto sum = 0;
	entityManager.for_each<int>(
			[&](entity_t ent, int i) {
			sum +=i;
			});
	std::cout << sum << "\n";
}
