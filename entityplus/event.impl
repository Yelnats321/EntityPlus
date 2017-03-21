//          Copyright Elnar Dakeshov 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

namespace entityplus {
#define EVENT_MANAGER_TEMPS \
template <typename... CTs, typename... TTs, typename... Es>

#define EVENT_MANAGER_SPEC \
event_manager<component_list<CTs...>, tag_list<TTs...>, Es...>

EVENT_MANAGER_TEMPS
template <typename Event>
void EVENT_MANAGER_SPEC::unsubscribe(detail::subscriber_handle_id_t id) {
	using EntityEvent = meta::not_<meta::typelist_has_type<Event, entity_events_t>>;
	auto er = meta::eval_if(
		[&](auto ident) {
			return std::get<detail::event_queue_t<Event>>(ident(eventQueues)).erase(id); 
		},
		meta::fail_cond<EntityEvent>([&](auto ident) {
			return ident(entityEventManager).template unsubscribe<Event>(id);
		})
	);
	(void)er; assert(er == 1);
}

EVENT_MANAGER_TEMPS
template <typename Event, typename Func>
subscriber_handle<Event> EVENT_MANAGER_SPEC::subscribe(Func &&func) {
	using ValidEvent = meta::typelist_has_type<Event, events_t>;
	using CanConstruct = std::is_constructible<detail::event_func_t<Event>, Func>;
	using EntityEvent = meta::not_<meta::typelist_has_type<Event, entity_events_t>>;
	return meta::eval_if(
		[&](auto id) {
			assert(std::numeric_limits<detail::subscriber_handle_id_t>::max() != currentId);
			auto sub = std::get<detail::event_queue_t<Event>>(id(eventQueues)).emplace(currentId++, std::forward<Func>(func));
			assert(sub.second);

			return subscriber_handle<Event>{*this, sub.first->first};
		},
		meta::fail_cond<ValidEvent>([](auto id) {
			static_assert(id(false), "subscribe called with invalid event");
			return std::declval<subscriber_handle<Event>>();
		}),
		meta::fail_cond<CanConstruct>([](auto id) {
			static_assert(id(false), "subscribe called with invalid callable");
			return std::declval<subscriber_handle<Event>>();
		}),
		meta::fail_cond<EntityEvent>([&](auto id) {
			return subscriber_handle<Event>{*this, id(entityEventManager).template subscribe<Event>(std::forward<Func>(func)) };
		})
	);
}

EVENT_MANAGER_TEMPS
template <typename Event>
void EVENT_MANAGER_SPEC::broadcast(const Event &event) const {
	using ValidEvent = meta::typelist_has_type<Event, custom_events_t>;
	meta::eval_if(
		[&](auto) {
			const auto &cont = std::get<detail::event_queue_t<Event>>(eventQueues);
			for (const auto &sub : cont) {
				sub.second(event);
			}
		},
		meta::fail_cond<ValidEvent>([](auto id) {
			static_assert(id(false), "broadcast called with invalid event");
		})
	);
}

#undef EVENT_MANAGER_TEMPS
#undef EVENT_MANAGER_SPEC
}
