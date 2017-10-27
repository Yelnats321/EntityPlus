//          Copyright Elnar Dakeshov 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <tuple>
#include <utility>
#include <vector>

#include "container.h"
#include "exception.h"
#include "metafunctions.h"
#include "typelist.h"

namespace entityplus {
enum class entity_status { UNINITIALIZED, DELETED, STALE, OK };

class entity_grouping;

template <typename...>
class event_manager;

struct control_block_t {
    bool breakout = false;
};

// Safety classes so that you can only create using the proper list types
template <typename Components, typename Tags>
class entity_manager {
    static_assert(meta::delay<Components, Tags>,
                  "The template parameters must be of type component_list and tag_list");
};

namespace detail {
    using entity_grouping_id_t = std::uintmax_t;

    template <typename, typename>
    struct entity_event_manager;

    template <typename Components, typename Tags>
    class entity {
        static_assert(meta::delay<Components, Tags>, "Don't create entities manually, use "
                                                     "entity_manager::entity_t or create_entity() "
                                                     "instead");
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
            explicit private_access() = default;
        };

        detail::entity_id_t id;
        entity_manager_t* entityManager = nullptr;
        mutable meta::type_bitset<comp_tag_t> compTags;

    public:
        entity() = default;

        entity(private_access, detail::entity_id_t id, entity_manager_t* entityManager) noexcept;

        entity_status get_status() const;

        template <typename Component>
        bool has_component() const;

        // Adds the component if it doesn't exist, otherwise returns the existing component
        template <typename Component, typename... Args>
        std::pair<Component&, bool> add_component(Args&&... args);
        template <typename Component>
        auto add_component(Component&& comp);

        // Returns if the component was removed or not (in the case that it didn't exist)
        template <typename Component>
        bool remove_component();

        // Must have component in order to get it, otherwise you have an invalid_component exception
        template <typename Component>
        Component& get_component();
        template <typename Component>
        const Component& get_component() const;

        template <typename Tag>
        bool has_tag() const;

        // Returns the previous tag value
        template <typename Tag>
        bool set_tag(bool set);

        // Updates entity to the stored one, returns false if deleted
        bool sync() const;

        void destroy();

        bool operator<(const entity& other) const noexcept;
        bool operator==(const entity& other) const noexcept;
    };
} // namespace detail

template <typename... Components, typename... Tags>
class entity_manager<component_list<Components...>, tag_list<Tags...>> {
public:
    using component_list_t = component_list<Components...>;
    using tag_list_t = tag_list<Tags...>;
    using entity_t = detail::entity<component_list_t, tag_list_t>;

private:
    using component_t = meta::typelist<Components...>;
    using tag_t = meta::typelist<Tags...>;
    using comp_tag_t = meta::typelist<Components..., Tags...>;
    using entity_container = flat_set<entity_t>;
    using entity_event_manager_t = detail::entity_event_manager<component_list_t, tag_list_t>;

    friend entity_t;
    friend entity_grouping;

    static_assert(meta::is_typelist_unique_v<comp_tag_t>,
                  "component_list and tag_list must not intersect");

    constexpr static auto ComponentCount = sizeof...(Components);
    constexpr static auto TagCount = sizeof...(Tags);
    constexpr static auto CompTagCount = ComponentCount + TagCount;

    detail::entity_id_t currentEntityId = 0;
    typename component_list_t::type components;
    entity_container entities;
    std::size_t maxLinearSearchDistance = 64;
    const entity_event_manager_t* eventManager = nullptr;
    detail::entity_grouping_id_t currentGroupingId = CompTagCount;
    flat_map<detail::entity_grouping_id_t,
             std::pair<meta::type_bitset<comp_tag_t>, entity_container>>
        groupings;

    [[noreturn]] void report_error(error_code errCode, const char* msg) const;

    std::pair<const entity_t*, entity_status> get_entity_and_status(const entity_t& entity) const;

    entity_t& assert_entity(const entity_t& entity);
    const entity_t& assert_entity(const entity_t& entity) const;

    template <typename T>
    void add_bit(entity_t& local, entity_t& foreign);

    template <typename T>
    void remove_bit(entity_t& local, entity_t& foreign);

    template <typename... Ts>
    std::pair<entity_container&, bool> get_smallest_container();

    template <typename Component, typename... Args>
    std::pair<Component&, bool> add_component(entity_t& entity, Args&&... args);

    template <typename... Ts>
    void add_components(entity_t& entity, Ts&&... ts);

    template <typename Component>
    bool remove_component(entity_t& entity);

    template <typename Component>
    const Component& get_component(const entity_t& entity) const;

    template <typename Tag>
    bool set_tag(entity_t& entity, bool set);

    // Expose this?
    template <typename... Ts>
    void set_tags(entity_t& entity, bool set);

    bool sync(const entity_t& entity) const;

    void destroy_entity(entity_t& entity);

    void destroy_grouping(detail::entity_grouping_id_t id);

public:
    using return_container = std::vector<entity_t>;

    entity_manager();
    entity_manager(const entity_manager&) = delete;
    entity_manager& operator=(const entity_manager&) = delete;

    template <typename... Ts, typename... Us>
    entity_t create_entity(Us&&... us);

    // Gets all entities that have the components and tags provided
    template <typename... Ts>
    return_container get_entities();

    template <typename... Ts, typename Func>
    void for_each(Func&& func);

    // No error handling (what if the grouping already exists?)
    template <typename... Ts>
    entity_grouping create_grouping();

    template <typename... Events>
    void set_event_manager(const event_manager<component_list_t, tag_list_t, Events...>& em);

    void clear_event_manager();

#ifdef ENTITYPLUS_NO_EXCEPTIONS
    using error_callback_t = void(error_code, const char*);

private:
    std::function<error_callback_t> errorCallback;

    [[noreturn]] void handle_error(error_code err, const char* msg) const;

public:
    void set_error_callback(std::function<error_callback_t> cb);
#endif
};

class entity_grouping {
    detail::entity_grouping_id_t id;
    void* manager = nullptr;
    void (*destroy_ptr)(entity_grouping*);

    template <typename CTs, typename TTs>
    static void destroy_impl(entity_grouping* self) {
        auto em = static_cast<entity_manager<CTs, TTs>*>(self->manager);
        em->destroy_grouping(self->id);
    }

    template <typename, typename>
    friend class entity_manager;
    struct private_access {
        explicit private_access() = default;
    };

public:
    entity_grouping() = default;

    template <typename CTs, typename TTs>
    entity_grouping(private_access, entity_manager<CTs, TTs>& em,
                    detail::entity_grouping_id_t id) noexcept;

    entity_grouping(entity_grouping&& other) noexcept;
    entity_grouping& operator=(entity_grouping&& other) noexcept;

    bool is_valid() const;

    bool destroy();
};
} // namespace entityplus

#include "entity.impl"
