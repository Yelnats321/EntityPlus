#include <entityplus/entity.h>
#include <iostream>

using namespace entityplus;

int main() {
	entity_manager<component_list<int>, tag_list<>> entityManager;
	for (int i = 0; i < 50; ++i) {
		entityManager.create_entity(i);
	}

	auto sum = 0;
	entityManager.for_each<int>(
			[&](auto ent, int i) {
			sum +=i;
			});
	std::cout << sum << "\n";
}
