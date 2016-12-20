#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <cstdint>
#include <boost/container/flat_map.hpp>

#include "TypeList.h"

namespace entityplus {
// Safety classes so that you can only create using the proper list types
// Subject to change for an additional customization point
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
	// Subject to change for an additional customization point
	using MyCompList = ComponentList<Components...>;
	using MyTagList = TagList<Tags...>;
	using MyEntityManager = EntityManager<MyCompList, MyTagList>;

	// For calling entity manipulation functions
	friend MyEntityManager;

	// The actual identifier
	detail::EntityID id_;
	// Version used for when there is an overwritten ID
	detail::EntityVersion version_;
	MyEntityManager *entityManager_;
	meta::type_bitset<Components...> components_;
	meta::type_bitset<Tags...> tags_;

	Entity(detail::EntityID id, detail::EntityVersion version, MyEntityManager *entityManager):
		id_(id), version_(version), entityManager_(entityManager) {}
public:
	template <class Component>
	bool hasComponent() const {
		return components_.at<Component>();
	}

	// Adds the component if it doesn't exist, otherwise returns the existing component
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
	static_assert(meta::is_tuple_unique<
				  std::tuple<
				  Components...,
				  Tags...
				  >
	>::value, "ComponentList and TagList must not intersect");

	// Subject to change for an additional customization point
	using MyCompList = ComponentList<Components...>;
	using MyTagList = TagList<Tags...>;
	using MyEntity = Entity<MyCompList, MyTagList>;
	friend MyEntity;
	using EntityContainer = boost::container::flat_set<MyEntity>;

	detail::EntityID currentId_;
	detail::EntityVersion version_;
	typename MyCompList::type components_;
	EntityContainer entities_;
	

	void assertEntity(const MyEntity &entity);

	template <class Component, typename... Args>
	std::pair<Component&, bool> addComponent(MyEntity &entity, Args&&... args);

	template <class Component>
	bool removeComponent(MyEntity &entity);

	template <class Component>
	Component & getComponent(const MyEntity &entity) {
		return const_cast<Component &>(static_cast<const decltype(this)>(this)->getComponent(entity));
	}

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

	MyEntity createEntity();
	void deleteEntity(const MyEntity &);

	// Gets all entities that have the components and tags provided
	template<typename... Ts>
	EntityContainer getEntities();
};

#include "Entity.impl"

}