#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <cstdint>

#include "TypeList.h"

namespace entityplus {

template <class Components, class Tags>
class EntityManager {
	static_assert(meta::delay<Components, Tags>::value,
				  "The template parameters must be of type ComponentList and TagList");
};
template <class Components, class Tags>
class Entity {
	static_assert(meta::delay<Components, Tags>::value,
				  "Don't create Entities manually, use EntityManager::createEntity() instead");
};

template <class... Components, class... Tags>
class Entity<ComponentList<Components...>, TagList<Tags...>> {
	using MyCompList = ComponentList<Components...>;
	using MyTagList = TagList<Tags...>;
	using MyEntityManager = EntityManager<MyCompList, MyTagList>;
	friend MyEntityManager;

	using EntityVersion = std::uintmax_t;

	detail::EntityID id_;
	EntityVersion version_;
	MyEntityManager * entityManager_;
	meta::type_bitset<Components...> components_;
	meta::type_bitset<Tags...> tags_;

	Entity(detail::EntityID id, EntityVersion version, MyEntityManager* entityManager):
		id_(id), version_(version), entityManager_(entityManager) {}
public:
	template <class Component>
	bool hasComponent() const {
		return components_.at<Component>();
	}

	// Returns the component, and if it was added or not (in the case it already exists)
	// If the component already exists, we reassign it to the new one created from the args
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

	using EntityContainer = std::vector<MyEntity>;

	typename MyEntity::EntityVersion version_;
	typename MyCompList::type components_;
	EntityContainer entities_;
	// can't use iterator because of invalidation
	typename EntityContainer::size_type newEntitiesItr_ = 0;
	

	void assertEntity(const MyEntity &entity);

	template <class Component, typename... Args>
	std::pair<Component&, bool> addComponent(MyEntity &entity, Args&&... args);

	template <class Component>
	bool removeComponent(MyEntity &entity);

	template <class Component>
	Component & getComponent(const MyEntity &entity);

	template <class Component>
	const Component & getComponent(const MyEntity &entity) const;

	template <class Tag>
	bool setTag(MyEntity &entity, bool set);
public:
	using EntityContainer = std::vector<MyEntity>;

	EntityManager() = default;
	EntityManager(const EntityManager &) = delete;
	EntityManager & operator = (const EntityManager &) = delete;
	EntityManager(EntityManager &&) = default;
	EntityManager & operator = (EntityManager &&) = default;

	// The entity is valid immediatly
	MyEntity createEntity();

	// The entity remains valid until step() is called
	void deleteEntity(const MyEntity &);

	// Executes all queued changes and updates all internal structures
	// Entities from before this point are considered invalid (but are not checked)
	void step();

	// Gets all entities that have the components and tags provided
	template<typename... Ts>
	EntityContainer getEntities();
};

#include "Entity.impl"

}