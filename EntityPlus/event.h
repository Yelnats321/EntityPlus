#pragma once

#include "metafunctions.h"

#include <tuple>
#include <vector>
#include <type_traits>
#include <functional>

namespace entityplus {

template <typename... Events>
class event_manager {
	using Events_t = meta::typelist<Events...>;
	static_assert(meta::is_typelist_unique_v<Events_t>,
				  "Events must be unique");
	template <typename T>
	using sig_t = void(const T &);
	template <typename T>
	using func_t = std::function<sig_t<T>>;
	template <typename T>
	using event_queue_t = std::vector<func_t<T>>;

	std::tuple<event_queue_t<Events>...> eventQueues_;
public:
	event_manager() = default;

	template <typename Event, typename Func>
	void register_handler(Func &&func) {
		using ValidEvent = meta::typelist_has_type<Event, Events_t>;
		using CanConstruct = std::is_constructible<func_t<Event>, Func>;
		meta::eval_if([&](auto) {
			std::get<event_queue_t<Event>>(eventQueues_).emplace_back(std::forward<Func>(func)); },
			meta::fail_cond<ValidEvent>([](auto delay) {
			static_assert(delay, "register_handler called with invalid event"); }),
			meta::fail_cond<CanConstruct>([](auto delay) {
			static_assert(delay, "register_handler called with invalid callable"); }));
	}

	template <typename Event>
	void broadcast(const Event &event) const {
		using ValidEvent = meta::typelist_has_type<Event, Events_t>;
		meta::eval_if([&](auto) {
			const auto &cont = std::get<event_queue_t<Event>>(eventQueues_);
			for (const auto &func : cont) func(event);
			},
			meta::fail_cond<ValidEvent>([](auto delay) {
			static_assert(delay, "broadcast called with invalid event"); }));
	}
};

}