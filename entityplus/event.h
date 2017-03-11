//          Copyright Elnar Dakeshov 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "metafunctions.h"
#include "container.h"

#include <tuple>
#include <type_traits>
#include <functional>
#include <cassert>

namespace entityplus {
namespace detail {
using subscriber_handle_id_t = std::uintmax_t;
}

template <typename... Events>
class event_manager;

template <typename Event>
class subscriber_handle {
	detail::subscriber_handle_id_t id;
	bool valid = false;
	void *manager;
	void (*unsubscribe_ptr)(subscriber_handle *, void *);

	template <typename... Events>
	static void unsubscribe_impl(subscriber_handle *self, void *manager) {
		auto em = static_cast<event_manager<Events...>*>(manager);
		return em->unsubscribe<Event>(self->id);
	}
public:
	subscriber_handle() = default;

	template <typename... Events>
	subscriber_handle(event_manager<Events...> &em, detail::subscriber_handle_id_t id) noexcept
		: id(id), valid(true), manager(&em), unsubscribe_ptr(&unsubscribe_impl<Events...>) {}

	subscriber_handle(subscriber_handle &&other) noexcept {
		*this = std::move(other);
	}

	subscriber_handle& operator=(subscriber_handle &&other) noexcept {
		id = other.id;
		valid = other.valid;
		manager = other.manager;
		unsubscribe_ptr = other.unsubscribe_ptr;

		other.valid = false;
		return *this;
	}

	bool is_valid() const {
		return valid;
	}

	bool unsubscribe() {
		if (valid) {
			unsubscribe_ptr(this, manager);
			valid = false;
			return true;
		}
		return false;
	}
};

template <typename... Events>
class event_manager {
	using events_t = meta::typelist<Events...>;
	static_assert(meta::is_typelist_unique_v<events_t>,
				  "Events must be unique");
	template <typename T>
	using sig_t = void(const T &);
	template <typename T>
	using func_t = std::function<sig_t<T>>;
	template <typename T>
	using event_queue_t = flat_map<detail::subscriber_handle_id_t, func_t<T>>;

	template <typename Event>
	friend class subscriber_handle;

	detail::subscriber_handle_id_t currentId = 0;
	std::tuple<event_queue_t<Events>...> eventQueues;

	template <typename Event>
	void unsubscribe(detail::subscriber_handle_id_t id) {
		auto er = std::get<event_queue_t<Event>>(eventQueues).erase(id);
		(void)er; assert(er == 1);
	}
public:
	event_manager() = default;
	event_manager(const event_manager &) = delete;
	event_manager& operator=(const event_manager &) = delete;

	template <typename Event, typename Func>
	subscriber_handle<Event> subscribe(Func &&func) {
		using ValidEvent = meta::typelist_has_type<Event, events_t>;
		using CanConstruct = std::is_constructible<func_t<Event>, Func>;
		return meta::eval_if([&](auto) {
			assert(std::numeric_limits<detail::subscriber_handle_id_t>::max() != currentId);
			auto sub = std::get<event_queue_t<Event>>(eventQueues).emplace(currentId++, std::forward<Func>(func));
			assert(sub.second);

			return subscriber_handle<Event>{*this, sub.first->first}; },
			meta::fail_cond<ValidEvent>([](auto delay) {
			static_assert(delay, "register_handler called with invalid event"); 
			return std::declval<subscriber_handle<Event>>(); }),
			meta::fail_cond<CanConstruct>([](auto delay) {
			static_assert(delay, "register_handler called with invalid callable");
			return std::declval<subscriber_handle<Event>>(); }));
	}

	template <typename Event>
	void broadcast(const Event &event) const {
		using ValidEvent = meta::typelist_has_type<Event, events_t>;
		meta::eval_if([&](auto) {
			const auto &cont = std::get<event_queue_t<Event>>(eventQueues);
			for (const auto &sub : cont) {
				sub.second(event);
			}},
			meta::fail_cond<ValidEvent>([](auto delay) {
			static_assert(delay, "broadcast called with invalid event"); }));
	}
};

}