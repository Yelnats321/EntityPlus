#pragma once

#include <vector>
#include <cstdint>
#include <tuple>
#include <utility>
#include <type_traits>
#include <array>
#include <boost/container/flat_set.hpp>
#include <unordered_set>

#include "typelist.h"
#include "metafunctions.h"
#include "exception.h"

namespace entityplus {
// Safety classes so that you can only create using the proper list types
template <typename Components, typename Tags>
class entity_manager {
	static_assert(meta::delay_v<Components, Tags>,
				  "The template parameters must be of type component_list and tag_list");
};

enum class entity_status {
	INVALID_MANAGER,
	NOT_FOUND,
	STALE,
	OK
};

namespace detail {
template <typename Components, typename Tags>
class entity {
	static_assert(meta::delay_v<Components, Tags>,
				  "Don't create entities manually, use entity_manager::create_entity() instead");
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
	struct private_access {};

	detail::entity_id_t id;
	entity_manager_t *entityManager;
	meta::type_bitset<comp_tag_t> compTags;
public:
	entity(private_access, detail::entity_id_t id, entity_manager_t *entityManager):
		id(id), entityManager(entityManager) {}

	entity_status get_status() const noexcept {
		return entityManager->get_entity_and_status(*this).second;
	}

	template <typename Component>
	inline bool has_component() const noexcept {
		using ValidComp = meta::typelist_has_type<Component, component_t>;
		return meta::eval_if(
			[&](auto) { return meta::get<Component>(compTags); },
			meta::fail_cond<ValidComp>([](auto delay) {
			static_assert(delay, "has_component called with invalid component");
			return false;
		}));
	}

	// Adds the component if it doesn't exist, otherwise returns the existing component
	template <typename Component, typename... Args>
	inline std::pair<Component&, bool> add_component(Args&&... args) {
		using ValidComp = meta::typelist_has_type<Component, component_t>;
		using IsConstructible = std::is_constructible<Component, Args&&...>;
		return meta::eval_if(
			[&](const std::false_type &) { return entityManager->template add_component<Component>(*this, std::forward<Args>(args)...); },
			meta::fail_cond<ValidComp>([](auto delay) {
			static_assert(delay, "add_component called with invalid component");
			return std::declval<std::pair<Component&, bool>>();}),
			meta::fail_cond<IsConstructible>([](auto delay) {
			static_assert(delay, "add_component cannot construct component with given args");
			return std::declval<std::pair<Component&, bool>>();
		}));
	}

	// Returns if the component was removed or not (in the case that it didn't exist)
	template <typename Component>
	inline bool remove_component() {
		using ValidComp = meta::typelist_has_type<Component, component_t>;
		return meta::eval_if(
			[&](auto) { return entityManager->template remove_component<Component>(*this); },
			meta::fail_cond<ValidComp>([](auto delay) {
			static_assert(delay, "remove_component called with invalid component");
			return false;
		}));
	}

	// Must have component in order to get it, otherwise you have a invalid_component exception
	template <typename Component>
	inline const Component& get_component() const {
		using ValidComp = meta::typelist_has_type<Component, component_t>;
		return meta::eval_if(
			[&](auto) -> decltype(auto) { return entityManager->template get_component<Component>(*this); },
			meta::fail_cond<ValidComp>([](auto delay) {
			static_assert(delay, "get_component called with invalid component");
			return std::declval<const Component &>();
		}));
	}
	template <typename Component>
	inline Component& get_component() {
		return const_cast<Component&>
			(meta::as_const(*this).template get_component<Component>());
	}

	template <typename Tag>
	inline bool has_tag() const noexcept {
		using ValidTag = meta::typelist_has_type<Tag, tag_t>;
		return meta::eval_if(
			[&](auto) { return meta::get<Tag>(compTags); },
			meta::fail_cond<ValidTag>([](auto delay) {
			static_assert(delay, "has_tag called with invalid tag");
			return false;
		}));
	}

	// returns the previous tag value
	template <typename Tag>
	inline bool set_tag(bool set) {
		using ValidTag = meta::typelist_has_type<Tag, tag_t>;
		return meta::eval_if(
			[&](auto) { return entityManager->template set_tag<Tag>(*this, set); },
			meta::fail_cond<ValidTag>([](auto delay) {
			static_assert(delay, "set_tag called with invalid tag");
			return false;
		}));
	}

	inline bool operator<(const entity &other) const {
		return id < other.id;
	}
	inline bool operator==(const entity &other) const {
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
	using entity_container = boost::container::flat_set<entity_t>;

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

	[[noreturn]] void report_error(error_code_t errCode, const char * error) const;

	std::pair<const entity_t*, entity_status> get_entity_and_status(const entity_t &entity) const noexcept;
#if !NDEBUG
	const entity_t & assert_entity(const entity_t &entity) const;
	entity_t & assert_entity(const entity_t &entity) {
		return const_cast<entity_t &>
			(meta::as_const(*this).assert_entity(entity));
	}
#endif

	template <typename Component, typename... Args>
	std::pair<Component&, bool> add_component(entity_t &entity, Args&&... args);

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

	template <typename... Ts>
	std::size_t get_smallest_idx() noexcept;
public:
	using return_container = std::vector<entity_t>;

	entity_manager() = default;
	entity_manager(const entity_manager &) = delete;
	entity_manager& operator = (const entity_manager &) = delete;

	entity_t create_entity();
	void delete_entity(const entity_t &);

	// Gets all entities that have the components and tags provided
	template<typename... Ts>
	return_container get_entities() noexcept;

	template<typename... Ts, typename Func>
	void for_each(Func && func);

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
}

#include "entity.impl"
