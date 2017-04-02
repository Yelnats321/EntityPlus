//          Copyright Elnar Dakeshov 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <tuple>
#include <type_traits>
#include <functional>
#include <cassert>

#include "metafunctions.h"
#include "container.h"
#include "typelist.h"
#include "entity.h"

namespace entityplus {
template <typename Entity>
struct entity_created {
	Entity entity;
};

template <typename Entity>
struct entity_destroyed {
	Entity entity;
};

template <typename Entity, typename Component>
struct component_added {
	Entity entity;
	Component &component;
};

template <typename Entity, typename Component>
struct component_removed {
	Entity entity;
	Component &component;
};

template <typename Entity, typename Tag>
struct tag_added {
	Entity entity;
};

template <typename Entity, typename Tag>
struct tag_removed {
	Entity entity;
};

namespace detail {
using subscriber_handle_id_t = std::uintmax_t;

template <typename T>
using event_sig_t = void(const T &);
template <typename T>
using event_func_t = std::function<event_sig_t<T>>;
template <typename T>
using event_queue_t = flat_map<subscriber_handle_id_t, event_func_t<T>>;

template <typename, typename>
class entity_event_manager;

template  <typename... Components, typename... Tags>
class entity_event_manager<component_list<Components...>, tag_list<Tags...>> {
	using entity_t = entity<component_list<Components...>, tag_list<Tags...>>;
public:
	using entity_created_t = entity_created<entity_t>;
	using entity_destroyed_t = entity_destroyed<entity_t>;
	template <typename T>
	using component_added_t = component_added<entity_t, T>;
	template <typename T>
	using component_removed_t = component_removed<entity_t, T>;
	template <typename T>
	using tag_added_t = tag_added<entity_t, T>;
	template <typename T>
	using tag_removed_t = tag_removed<entity_t, T>;
private:
	using entity_events_t = meta::typelist<
		entity_created_t,
		entity_destroyed_t,
		component_added_t<Components>...,
		component_removed_t<Components>...,
		tag_added_t<Tags>...,
		tag_removed_t<Tags>...
	>;

	static_assert(meta::is_typelist_unique_v<entity_events_t>,
				  "Internal events not unique. Panic!");

	detail::subscriber_handle_id_t currentId = 0;
	meta::tuple_from_typelist_t<entity_events_t, event_queue_t> eventQueues;

	template <typename...>
	friend class ::entityplus::event_manager;

	friend class ::entityplus::entity_manager<component_list<Components...>, tag_list<Tags...>>;

	template <typename Event>
	void broadcast(const Event &event) const {
		static_assert(meta::typelist_has_type_v<Event, entity_events_t>, "broadcast called with invalid event");
		const auto &cont = std::get<event_queue_t<Event>>(eventQueues);
		for (const auto &sub : cont) sub.second(event);
	}

	template <typename Event, typename Func>
	subscriber_handle_id_t subscribe(Func &&func) {
		assert(std::numeric_limits<subscriber_handle_id_t>::max() != currentId);
		auto sub = std::get<event_queue_t<Event>>(eventQueues).emplace(currentId++, std::forward<Func>(func));
		assert(sub.second);

		return sub.first->first;
	}

	template <typename Event>
	auto unsubscribe(detail::subscriber_handle_id_t id) {
		return std::get<event_queue_t<Event>>(eventQueues).erase(id);
	}
};
} // namespace detail

template <typename... Events>
class event_manager;

template <typename Event>
class subscriber_handle {
	detail::subscriber_handle_id_t id;
	void *manager = nullptr;
	void (*unsubscribe_ptr)(subscriber_handle *);

	template <typename... Events>
	static void unsubscribe_impl(subscriber_handle *self) {
		auto em = static_cast<event_manager<Events...>*>(self->manager);
		em->template unsubscribe<Event>(self->id);
	}
public:
	subscriber_handle() = default;

	template <typename... Events>
	subscriber_handle(event_manager<Events...> &em, detail::subscriber_handle_id_t id) noexcept
		: id(id), manager(&em), unsubscribe_ptr(&unsubscribe_impl<Events...>) {}

	subscriber_handle(subscriber_handle &&other) noexcept {
		*this = std::move(other);
	}

	subscriber_handle& operator=(subscriber_handle &&other) noexcept {
		id = other.id;
		manager = other.manager;
		unsubscribe_ptr = other.unsubscribe_ptr;

		other.manager = nullptr;
		return *this;
	}

	bool is_valid() const {
		return manager != nullptr;
	}

	bool unsubscribe() {
		if (manager) {
			unsubscribe_ptr(this);
			manager = nullptr;
			return true;
		}
		return false;
	}
};

template <typename... Components, typename... Tags, typename... Events>
class event_manager<component_list<Components...>, tag_list<Tags...>, Events...> {
	using custom_events_t = meta::typelist<Events...>;
	static_assert(meta::is_typelist_unique_v<custom_events_t>,
				  "Events must be unique");
	using entity_event_manager_t = detail::entity_event_manager<component_list<Components...>, tag_list<Tags...>>;
	using entity_events_t = typename entity_event_manager_t::entity_events_t;
	using events_t = meta::typelist_concat_t<custom_events_t, entity_events_t>;
	static_assert(meta::is_typelist_unique_v<events_t>,
				  "Do not specify the predefined events");

	template <typename Event>
	friend class subscriber_handle;

	friend class entity_manager<component_list<Components...>, tag_list<Tags...>>;

	detail::subscriber_handle_id_t currentId = 0;
	std::tuple<detail::event_queue_t<Events>...> eventQueues;
	entity_event_manager_t entityEventManager;

	template <typename Event>
	void unsubscribe(detail::subscriber_handle_id_t id);

	const auto & get_entity_event_manager() const {
		return entityEventManager;
	}
public:
	event_manager() = default;
	event_manager(const event_manager &) = delete;
	event_manager& operator=(const event_manager &) = delete;

	template <typename Event, typename Func>
	subscriber_handle<Event> subscribe(Func &&func);

	template <typename Event>
	void broadcast(const Event &event) const;
};

}

#include "event.impl"
