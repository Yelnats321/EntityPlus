//          Copyright Elnar Dakeshov 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstdint>
#include <functional>
#include <tuple>

#include "container.h"
#include "entity.h"
#include "metafunctions.h"
#include "typelist.h"

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
    Component& component;
};

template <typename Entity, typename Component>
struct component_removed {
    Entity entity;
    Component& component;
};

template <typename Entity, typename Tag>
struct tag_added {
    Entity entity;
};

template <typename Entity, typename Tag>
struct tag_removed {
    Entity entity;
};

template <typename Event>
class subscriber_handle;

template <typename... Ts>
class event_manager {
    static_assert(meta::delay<Ts...>,
                  "The first two template paramaters must be of type component_list and type_list");
};

namespace detail {
    using subscriber_handle_id_t = std::uintmax_t;

    template <typename T>
    using event_sig_t = void(const T&);
    template <typename T>
    using event_func_t = std::function<event_sig_t<T>>;
    template <typename T>
    using event_map_t = flat_map<subscriber_handle_id_t, event_func_t<T>>;

    template <typename, typename>
    struct entity_event_manager;

    // Does not need to be own struct, can exist inside the event_manager
    template <typename... Components, typename... Tags>
    struct entity_event_manager<component_list<Components...>, tag_list<Tags...>> {
        using entity_t = entity<component_list<Components...>, tag_list<Tags...>>;
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
        using entity_events_t =
            meta::typelist<entity_created_t, entity_destroyed_t, component_added_t<Components>...,
                           component_removed_t<Components>..., tag_added_t<Tags>...,
                           tag_removed_t<Tags>...>;

        static_assert(meta::is_typelist_unique_v<entity_events_t>,
                      "Internal events not unique. Panic!");

        detail::subscriber_handle_id_t currentId = 0;
        meta::tuple_from_typelist_t<entity_events_t, event_map_t> eventQueues;

        template <typename Event>
        void broadcast(const Event& event) const;

        template <typename Event, typename Func>
        subscriber_handle_id_t subscribe(Func&& func);

        template <typename Event>
        auto unsubscribe(detail::subscriber_handle_id_t id);
    };
} // namespace detail

template <typename... Components, typename... Tags, typename... Events>
class event_manager<component_list<Components...>, tag_list<Tags...>, Events...> {
    using custom_events_t = meta::typelist<Events...>;
    static_assert(meta::is_typelist_unique_v<custom_events_t>, "Events must be unique");
    using entity_event_manager_t =
        detail::entity_event_manager<component_list<Components...>, tag_list<Tags...>>;
    using entity_events_t = typename entity_event_manager_t::entity_events_t;
    using events_t = meta::typelist_concat_t<custom_events_t, entity_events_t>;
    static_assert(meta::is_typelist_unique_v<events_t>, "Do not specify native events");

    template <typename Event>
    friend class subscriber_handle;

    friend class entity_manager<component_list<Components...>, tag_list<Tags...>>;

    detail::subscriber_handle_id_t currentId = 0;
    std::tuple<detail::event_map_t<Events>...> eventQueues;
    entity_event_manager_t entityEventManager;

    template <typename Event>
    void unsubscribe(detail::subscriber_handle_id_t id);

    const auto& get_entity_event_manager() const;

public:
    event_manager() = default;
    event_manager(const event_manager&) = delete;
    event_manager& operator=(const event_manager&) = delete;

    template <typename Event, typename Func>
    subscriber_handle<Event> subscribe(Func&& func);

    template <typename Event>
    void broadcast(const Event& event) const;
};

template <typename Event>
class subscriber_handle {
    detail::subscriber_handle_id_t id;
    void* manager = nullptr;
    void (*unsubscribe_ptr)(subscriber_handle*);

    template <typename... Events>
    static void unsubscribe_impl(subscriber_handle* self);

    template <typename...>
    friend class event_manager;

    struct private_access {
        explicit private_access() = default;
    };

public:
    subscriber_handle() = default;

    template <typename... Events>
    subscriber_handle(private_access, event_manager<Events...>& em,
                      detail::subscriber_handle_id_t id) noexcept;

    subscriber_handle(subscriber_handle&& other) noexcept;
    subscriber_handle& operator=(subscriber_handle&& other) noexcept;

    bool is_valid() const;

    bool unsubscribe();
};
} // namespace entityplus

#include "event.impl"
