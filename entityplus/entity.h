//          Copyright Elnar Dakeshov 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <vector>
#include <cstdint>
#include <tuple>
#include <utility>
#include <type_traits>
#include <array>
#include <functional>
#include <cassert>

#include "typelist.h"
#include "metafunctions.h"
#include "exception.h"
#include "container.h"

namespace entityplus {
// Safety classes so that you can only create using the proper list types
template <typename Components, typename Tags>
class entity_manager {
	static_assert(meta::delay_v<Components, Tags>,
				  "The template parameters must be of type component_list and tag_list");
};

enum class entity_status {
	UNINITIALIZED,
	DELETED,
	STALE,
	OK
};

template <typename...>
class event_manager;

namespace detail {
template <typename Components, typename Tags>
class entity_event_manager;

template <typename Components, typename Tags>
class entity {
	static_assert(meta::delay_v<Components, Tags>,
				  "Don't create entities manually, use entity_manager::entity_t or create_entity() instead");
};

template <typename... Components, typename... Tags>
class entity<component_list<Components...>, tag_list<Tags...>> {
public:
	using component_list_t = component_list<Components...>;
	using tag_list_t = tag_list<Tags...>;
	using entity_manager_t = entity_manager<component_list_t, tag_list_t>;
private:
	using component_t = meta::typelist<Components...>;
	using tag_t = meta::typelist<Tags...>;
	using comp_tag_t = meta::typelist<Components..., Tags...>;

	friend entity_manager_t;
	struct private_access {
		explicit private_access() {}
	};

	detail::entity_id_t id;
	entity_manager_t *entityManager = nullptr;
	meta::type_bitset<comp_tag_t> compTags;
public:
	entity() = default;

	entity(private_access, detail::entity_id_t id, entity_manager_t *entityManager) noexcept
		: id(id), entityManager(entityManager) {}

	entity_status get_status() const {
		if (!entityManager) return entity_status::UNINITIALIZED;
		return entityManager->get_entity_and_status(*this).second;
	}

	template <typename Component>
	bool has_component() const;

	// Adds the component if it doesn't exist, otherwise returns the existing component
	template <typename Component, typename... Args>
	std::pair<Component&, bool> add_component(Args&&... args);

	// Returns if the component was removed or not (in the case that it didn't exist)
	template <typename Component>
	bool remove_component();

	// Must have component in order to get it, otherwise you have a invalid_component exception
	template <typename Component>
	const Component& get_component() const;
	// Must have component in order to get it, otherwise you have a invalid_component exception
	template <typename Component>
	Component& get_component() {
		return const_cast<Component&>
			(meta::as_const(*this).template get_component<Component>());
	}

	template <typename Tag>
	bool has_tag() const;

	// Returns the previous tag value
	template <typename Tag>
	bool set_tag(bool set);

	// Updates entity to the stored one, returns false if deleted
	bool sync() {
		assert(entityManager);
		return entityManager->sync(*this);
	}

	void destroy() {
		assert(entityManager);
		entityManager->destroy_entity(*this);
	}

	inline bool operator<(const entity &other) const {
		assert(entityManager && entityManager == other.entityManager);
		return id < other.id;
	}
	inline bool operator==(const entity &other) const {
		assert(entityManager && entityManager == other.entityManager);
		return id == other.id;
	}
};
}

template <typename... Components, typename... Tags>
class entity_manager<component_list<Components...>, tag_list<Tags...>> {
public:
	using component_list_t = component_list<Components...>;
	using tag_list_t = tag_list<Tags...>;
	using entity_t = detail::entity<component_list_t, tag_list_t>;
private:
	using component_t = meta::typelist<Components... >;
	using tag_t = meta::typelist<Tags...>;
	using comp_tag_t = meta::typelist<Components..., Tags...>;
	using entity_container = flat_set<entity_t>;
	using entity_event_manager_t = detail::entity_event_manager<component_list_t, tag_list_t>;

	friend entity_t;

	static_assert(meta::is_typelist_unique_v<comp_tag_t>,
				  "component_list and tag_list must not intersect");

	constexpr static auto ComponentCount = sizeof...(Components);
	constexpr static auto TagCount = sizeof...(Tags);
	constexpr static auto CompTagCount = ComponentCount + TagCount;

	detail::entity_id_t currentId = 0;
	typename component_list_t::type components;
	entity_container entities;
	std::array<entity_container, CompTagCount> entityCount;
	std::size_t maxLinearSearchDistance = 64;
	const entity_event_manager_t *eventManager = nullptr;

	[[noreturn]] void report_error(error_code_t errCode, const char * error) const;

	std::pair<const entity_t*, entity_status> get_entity_and_status(const entity_t &entity) const;
#if !NDEBUG
	const entity_t & assert_entity(const entity_t &entity) const;
	entity_t & assert_entity(const entity_t &entity) {
		return const_cast<entity_t &>
			(meta::as_const(*this).assert_entity(entity));
	}
#endif

	template <typename Component, typename... Args>
	std::pair<Component&, bool> add_component(entity_t &entity, const std::tuple<Args...> &args);

	template <typename Component>
	bool remove_component(entity_t &entity);

	template <typename Component>
	const Component& get_component(const entity_t &entity) const;

	template <typename Tag>
	bool set_tag(entity_t &entity, bool set);

	template <typename T>
	void add_bit(entity_t &local, entity_t &foreign);

	template <typename T>
	void remove_bit(entity_t &local, entity_t &foreign);

	bool sync(entity_t &entity) const;

	void destroy_entity(const entity_t &entity);

	template <typename... Ts>
	entity_container& get_smallest_idx();
public:
	using return_container = std::vector<entity_t>;

	entity_manager() = default;
	entity_manager(const entity_manager &) = delete;
	entity_manager& operator=(const entity_manager &) = delete;

	entity_t create_entity();

	// Gets all entities that have the components and tags provided
	template<typename... Ts>
	return_container get_entities();

	template<typename... Ts, typename Func>
	void for_each(Func && func);

	std::size_t get_max_linear_dist() const {
		return maxLinearSearchDistance;
	}

	void set_max_linear_dist(std::size_t maxLinearDist) {
		return maxLinearSearchDistance = maxLinearDist;
	}

	template <typename... Events>
	void set_event_manager(const event_manager<component_list_t, tag_list_t, Events...> &em);

	void clear_event_manager() {
		eventManager = nullptr;
	}

#ifdef ENTITYPLUS_NO_EXCEPTIONS
	using error_callback_t = void(error_code_t, const char *);
private:
	std::function<error_callback_t> errorCallback;

	[[noreturn]] void handle_error(error_code_t err, const char *msg) const {
		if (errorCallback) errorCallback(err, msg);
		std::terminate();
	}
public:
	void set_error_callback(std::function<error_callback_t> cb) {
		errorCallback = std::move(cb);
	}
#endif
};

struct control_block_t {
	bool breakout = false;
};
}

#include "entity.impl"
