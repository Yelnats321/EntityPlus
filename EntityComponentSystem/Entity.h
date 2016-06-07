#pragma once

#include <unordered_map>
#include <unordered_set>
//#include <vector>
#include <bitset>
#include <stdexcept>

namespace entityplus {

using EntityID = std::size_t;

namespace detail {
using EntityVersion = std::size_t;
}

class invalid_component : public std::logic_error {
public:
	using std::logic_error::logic_error;
};

template <class Components, class Tags>
class EntityManager {
	static_assert(meta::delay<Components, Tags>::value,
				  "The template parameters must be of type ComponentList and TagList");
};
template <class Components, class Tags>
class Entity {
	static_assert(meta::delay<Components, Tags>::value,
				  "Don't create Entities manually, use EntityManager::createEntity instead");
};

template <typename... Ts>
struct TagList {
	template <typename T>
	struct containerType : public std::unordered_set<EntityID> {};
	using type = std::tuple<containerType<Ts>...>;
	static_assert(meta::is_tuple_unique<std::tuple<Ts...>>::value, "TagList must be unique");
	static constexpr auto size = sizeof...(Ts);
};

template <typename... Ts>
struct ComponentList {
	template <typename T>
	using containerType = std::unordered_map<EntityID, T>;
	using type = std::tuple<containerType<Ts>...>;
	static_assert(meta::is_tuple_unique<std::tuple<Ts...>>::value, "ComponentList must be unique");
	static constexpr auto size = sizeof...(Ts);
};

template <class... Components, class... Tags>
class Entity<ComponentList<Components...>, TagList<Tags...>> {
	using MyCompList = ComponentList<Components...>;
	using MyTagList = TagList<Tags...>;
	using MyEntityManager = EntityManager<MyCompList, MyTagList>;
	friend MyEntityManager;

	EntityID id_;
	detail::EntityVersion version_;
	MyEntityManager * entityManager_;
	meta::type_bitset<Components...> components_;
	meta::type_bitset<Tags...> tags_;

	Entity(EntityID id, detail::EntityVersion version, MyEntityManager* entityManager):
		id_(id), version_(version), entityManager_(entityManager) {}
public:
	template <class Component>
	bool hasComponent() const {
		return components_.at<Component>();
	}

	// Returns the component, and if it was added or not (in the case it already exists)
	template <class Component, typename... Args>
	std::pair<Component&, bool> addComponent(Args&&... args) {
		return entityManager_->addComponent<Component>(*this, std::forward<Args>(args)...);
	}

	// Returns if the component was removed or not (in the case that it didn't exist)
	template <class Component>
	bool removeComponent() {
		return entityManager_->removeComponent<Component>(*this);
	}

	// Must have component in order to get it, otherwise you have a invalid_component exception
	template <class Component>
	Component& getComponent() {
		return entityManager_->getComponent<Component>(*this);
	}

	template <class Component>
	const Component& getComponent() const {
		return entityManager_->getComponent<Component>(*this);
	}

	template <class Tag>
	bool hasTag() const {
		return tags_.at<Tag>();
	}

	// returns the previous tag value
	template <class Tag>
	bool setTag(bool set) {
		return entityManager_->setTag<Tag>(*this, set);
	}
};


template <class... Components, class... Tags>
class EntityManager<ComponentList<Components...>, TagList<Tags...>> {
	using MyCompList = ComponentList<Components...>;
	using MyTagList = TagList<Tags...>;
	using MyEntity = Entity<MyCompList, MyTagList>;
	friend MyEntity;

	static_assert(meta::is_tuple_unique<
				  std::tuple<
				  Components...,
				  Tags...
				  >
	>::value, "ComponentList and TagList must not intersect");

	template <class Component, typename... Args>
	std::pair<Component&, bool> addComponent(MyEntity &entity, Args&&... args);

	template <class Component>
	bool removeComponent(MyEntity &entity);

	template <class Tag>
	bool setTag(MyEntity &entity, bool set);
public:
	using EntityContainer = std::vector<Entity>;

	EntityManager() = default;

	// The entity is valid immediatly
	MyEntity createEntity();

	// The entity remains valid until step() is called
	void deleteEntity(const MyEntity &);

	// Deletes all entities and updates all internal structures
	// Entities from before this point are considered invalid (but are not checked)
	void step();

	// Gets all entities that have the components and tags provided
	template<typename... Ts>
	EntityContainer getEntities();
};

#include "Entity.impl"

}