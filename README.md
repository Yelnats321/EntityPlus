# EntityPlus
EntityPlus is an Entity Component System library written in C++14, offering fast compilation and runtime speeds. The library is header only, saving you the trouble of fidgeting with build systems and there are no external dependencies.

The ECS framework is an attempt to decouple data from mechanics. In doing so, it lets you create objects out of building blocks that mesh together to create a whole. It models a has-a relationship, letting you expand without worrying about dependency trees and inheritance. The three main aspects of an ECS framework are of course Entities, Components, and Systems.

## Requirements
EntityPlus requires C++14 conformance, and was mainly developed on MSVC. It has been tested to work on

* MSVC 2015 update 3
* Clang 3.5.0
* GCC 5.3.0

### Components
Components contain information. This can be anything, such as health, a piece of armor, or a status effect. An example component could be the identity of a person, which could be modeled like this:
```c++
struct identity {
    std::string name_;
    int age_;
    identity(std::string name, int age) : name_(name), age_(age) {}
};
```
Components don't have to be aggregate types, they can be as complicated as they need to be. For example, if we wanted a health component that would only let you heal to a maximum health, we could do it like this:
```c++
class health {
    int health_, maxHealth_;
public:
    health(int health, int maxHealth) :
        health_(health), maxHealth_(maxHealth){}
    
    int addHealth(int health) {
        return health_ = std::max(health+health_, maxHealth);
    }
}
```
As you may have noticed, these are just free classes. Usually, to use them with other ECSs you'd have to make them inherit from some common base, probably along with CRTP. However, EntityPlus takes advantage of the type system to eliminate these needs. To use these classes we have to simply specify them later.

Components must have a constructor, so aggregates are not allowed. This restriction is the same as all `emplace()` methods in the standard library. There are no other requirements.

### Entities
Entities model something. You can think of them as containers for components. If you want to model a player character, you might want a name, a measurement of their health, and an inventory. To use an entity, we must first create it. However, you can't just create a standalone `entity`, it needs context on where it exists. We use an `entity_manager` to manage all our `entity`s for us.
```c++
using CompList = component_list<identity, health>;
using TagList = tag_list<struct TA, struct TB>;
entity_manager<CompList, TagList> entityManager;
using entity_t = typename entity_manager<CompList, TagList>::entity_t;
```
Don't be scared by the syntax. Since we don't rely on inheritance or CRTP, we must give the `entity_manager` the list of components we will use with it, as well as a list of [tags](#tags). To create a list of components, we simply use a `component_list`. `component_list`s and `tag_list`s have to be unique, and the `component_list` and `tag_list` can't have overlapping types. If you mess up, you'll be told via compiler error.
```c++
error C2338: component_list must be unique
```
Not so bad, right? EntityPlus is designed with the end user in mind, attempting to achieve as little template error bloat as possible. Almost all template errors will be reported in a simple and concise manner, with minimal errors as the end goal. With `C++17` most code will switch over to using `constexpr if` for errors, which will reduce the error call stack even further.

Now that we have a manager, we can create an actual entity.
```c++
entity_t entity = entityManager.create_entity();
```
You probably want to add those components to the entity.
```c++
auto retId = entity.add_component<identity>("John", 25);
retId.first.name_ = "Smith";
entity.add_component(health{100, 100});
```
If we supply a component that wasn't part of the original component list, we will be told this at compile time. In fact, any sort of type mismatch will be presented as a user friendly error when you compile. Adding a component is quite similar to using `map::emplace()`, because the function forwards its args to the constructor and has a similar return semantic. A `pair<component&, bool>` is returned, indicating error or success and the component. The function can fail if a component of that type already exists, in which case the returned `component&` is a reference to the already existing component. Otherwise, the function succeeded and the new component is returned.

Sometimes you know all the tags and components you want from the get go. You can create an entity with all these parts just as easily:

```c++
entity_t ent = entityManager.create_entity<TA>(A{3}, health{100, 200});
```

The arguments are fully formed components you wish to add to the entity and the template arguments are the tags the entity should have once it's created.

What happens if we create a copy of an entity? Well, since entities are just handles, this copy doesn't represent a new entity but instead refers to the same underlying data that you copied.

```c++
auto entityCopy = entity;
assert(entityCopy.get_component<health>() == entity.get_component<health>();
```

What happens if we modify one copy of the entity? Well, the modified entity is the freshest, and so it is fine, but the old entity is stale. Using a stale entity will give you an error at best, but it can go unnoticed under certain circumstances (if using a release build). You can query the state of an entity with `get_status()`. The 4 statuses are OK, stale, deleted, and uninitialized. To make sure you have the newest version of an entity, you can use `sync()`, which will update your entity to the latest version. If the entity has been deleted, `sync()` will return false.

### Systems
The last thing we want to do is manipulate our entities. Unlike some ECS frameworks, EntityPlus doesn't have a system manager or similar device. You can work with the entities in one of two ways. The first is querying for a list of them by type
```c++
auto ents = entityManager.get_entities<identity>();
for (const auto &ent : ents) {
    std::cout << ent.get_component<identity>().name_ << "\n";
}
```
`get_entities()` will return a `vector` of all the entities that contain the given components and tags.

The second, and faster way, of manipulating entities is by using lambdas (or any `Callable` really).
```c++
entityManager.for_each<identity>([](auto ent, auto &id) {
    std::cout << id.name_ << "\n";
}
```

You can supply as many tags/components as you want to both methods, so if you need all entities with `tag1` and `tag2` you can simply do `get_entities<tag1, tag2>()` or `for_each<tag1, tag2>(...)`. In addition, `for_each` has an optional control parameter, which you can modify to break out of the for loop early.

```c++
entity_t secretAgent;
entityManager.for_each<tag1, identity>([&](auto ent, auto &id, control_block_t &control) {
	if (id.name_ == "Secret Agent") {
		secretAgent = ent;
		control.breakout = true;
	}
}
```

That's about it! You can obviously wrap these methods in your own system classes, but having specific support for systems felt artificial and didn't add any impactful or useful changes to the flow of usage.

### Tags
Tags are like components that have no data. They are simply a typename (and don't even have to be complete types) that is attached to an entity. An example could be a player tag for the entity that is controlled by a player. Tags can be used in any way a component is, but since there is no value associated with it except if it exists or not, it can only be toggled.

```c++
ent.set_tag<player_tag>(true);
assert(ent.get_tag<player_tag>() == true);
```

### Events
Events are orthogonal to ECS, but when used in conjunction they create better decoupled code. Because of this, events are fully integrated into the entity manager. The first two template arguments of the `event_manager` must be the same `component_list` and `tag_list` as the ones used for the `entity_manager`. Additional events can be used by supplying their type after the components/tags.

```c++
struct MyCustomEvent {
	std::string msg;
};

event_manager<CompList, TagList, MyCustomEvent> eventManager;
subscriber_handle<entity_created> handle;
handle = eventManager.subscribe<MyCustomEvent>([](const auto &ev) {
    std::cout << ev.msg;
});
eventManager.broadcast(MyCustomEvent{"surprise!"});
handle.unsubscribe();
```

Subscriber handles are ways of keeping track of a subscribers. They do not rely on the type of event manager, unlike entities, and can be stored just like any other object. They do not get invalidated either.

There are also special events that are generated by the entity manager, which is why we need the components/tags to be the same. To use an event manager with an entity manager, you must set it.

```c++
entityManager.set_event_manager(eventManager);
```

By doing this, you can now be notified to a wide variety of state changes. For example, if you want to know whenever a component of type `health` is added to an entity, you can simply subscribe to that event.

```c++
eventManager.subscribe<component_added<entity_t, health>>([](const auto &event) {
	event.ent.get_component<health>() == event.component;
}
```

Here is a full list of predefined events.

```
entity_created<entity_t>
entity_destroyed<entity_t>
component_added<entity_t, Component>
component_removed<entity_t, Component>
tag_added<entity_t, Tag>
tag_remved<entity_t, Tag>
```

Note that a destructive event is issued at the earliest possible point while a constructive event is issued at the latest. This is so that you can use as much information inside the event handler as possible. It is rarely useful to know if an entity was destroyed if you can't access any of its components, and it is likewise useless to know that a component was added to an entity before the component exists. Component/tag removal events are also issued when an entity is being destroyed, however they are issued after an entity destroyed event for the aforementioned reason.


### Exceptions and Error Codes
EntityPlus can be configured to use either exceptions or error codes. The two types of exceptions are `invalid_component` and `bad_entity`, with corresponding error codes. The former is thrown when `get_component()` is called for an entity that does not own a component of that type. The latter is thrown when an entity is stale, belongs to another entity manager, or when the entity has already been deleted. These states can be queried by `get_status()` which returns a corresponding `entity_status`.

To enable error codes, you must `#define ENTITYPLUS_NO_EXCEPTIONS` and `set_error_callback()`, which takes a `std::function<void(error_code_t code, const char *msg)>` as an argument.

## Performance
EntityPlus was designed with performance in mind. Almost all information is stored contiguously (through `flat_map`s and `flat_set`s) and the code has been optimized for iteration (over insertion/deletion) as that is the most common operation when using ECS. 

There is currently little tuning available, but additional enhancements are planned. Entity manager provides a `set_max_linear_dist` option. When iterating using `for_each`, the relative occurrence of a component is calculated against the maximum possible amounts of iterated entities. If this number is small, then a  linear search is used to find consecutive entities. Otherwise, a binary search is used instead.

For example, say there are 1000 entities. 500 of these entities have component A, but only 10 have component B. We call `for_each<A,B>`. Since we know the maximum amount of entities we will iterate is 10, we calculate the relative occurrence of these entities in `A`. 500/10 = 50, which means we are likely to iterate over 50 entities in a linear search before we find an entity that has both `A` and `B`. If we had `set_max_linear_dist(55)` then we would do a linear search through the entities that contain `A`. If instead we had `set_max_linear_dist(25)`, we would do a binary search. The default value is 64, and can be queried with `get_max_linear_dist()`.

### Groups
If you know you will be querying some set of components/tags often, you can register an entity group. This means that under the hood, the entity manager will keep all entities with the components/tags together in a container so that when you need to iterate over the grouping it won't have to generate it on the fly. For example, if you know you will use components `A` and `B` together in a system, you can do this:
```c++
entity_grouping groupAB = entityManager.create_grouping<A, B>();

for_each<A,B>(...);

// later
groupAB.destroy()
```
Now whenever you do a `for_each<A,B>()` or a `get_entities<A,B>()` the iterated entities will not have to be built dynamically but are already cached. Additionally, whenever you do a query like `for_each<A,B,C>()` the manager will only iterate through the smallest subset of tags/components it can find, which in this case would be the group `AB`, so you will get performance gains through that as well.

There are already pre-generated groupings for each component and tag, so you cannot create a grouping with an 0 or 1 items (since 0 is just every entity and 1 is just a single component/tag).

### Benchmarks
I've benchmarked EntityPlus against EntityX, another ECS library for C++11 on my Lenovo Y-40 which has an i7-4510U @ 2.00 GHz. Compiled using MSVC 2015 update 3 with hotfix on x64. The source for the benchmarks can be viewed [here](entityplus/benchmark.cpp). The time to add the components was very negligible and unlikely to impact performance much in the long run unless you're adding/removing components more than you are iterating over them.

```
Entity Count | Iterations | Probability | EntityPlus | EntityX
----------------------------------------------------------------
    1 000    | 1 000 000  |    1 in 3   |   1706 ms  |  20959 ms
   10 000    | 1 000 000  |    1 in 3   |  16541 ms  | 208156 ms
   30 000    |   100 000  |    1 in 3   |   5308 ms  |  63012 ms
  100 000    |   100 000  |    1 in 5   |  14780 ms  | 133365 ms
   10 000    | 1 000 000  |  1 in 1000  |    396 ms  |  16883 ms
  100 000    | 1 000 000  |  1 in 1000  |   4610 ms  | 170271 ms
   
```

### Big O Analysis
```c++
n = amount of entities

Entity:
has_(component/tag) = O(1)
(add/remove)_component = O(n)
set_tag = O(n)
get_component = O(log n)
get_status = O(log n)
sync = O(log n)
destroy = O(n)

Entity Manager:
create_entity = O(n)
get_entities = O(n)
for_each = O(n)
create_grouping = O(n)
```

## Reference
### Entity
```c++
entity_status get_status() const 
```
`Returns`: Status of `entity`, one of `OK`, `UNINITIALIZED`, `DELETED`, or `STALE`.

```c++
template <typename Component>
bool has_component() const 
```
`Returns`: `bool` indicating whether the `entity` has the `Component`. 

`Prerequisites`: `entity` is `OK`.

```c++
template <typename Component, typename... Args>
std::pair<Component&, bool> add_component(Args&&... args)

template <typename Component>
std::pair<std::decay_t<Component>&, bool> add_component(Component&& comp)
```
`Returns`: `bool` indicating if the `Component` was added. If it was, a reference to the new `Component`. Otherwise, the old `Component`. Does not overwrite old `Component`.

`Prerequisites`: `entity` is `OK`.

`Throws`: `bad_entity` if the `entity` is not `OK`.

Can invalidate references to all components of type `Component`, as well as a `for_each` involving `Component`.

Can turn entity copies `STALE`.

```c++
template <typename Component>
bool remove_component()
```
`Returns`: `bool` indicating if the `Component` was removed.

`Prerequisites`: `entity` is `OK`.

Can invalidate references to all components of type `Component`, as well as a `for_each` involving `Component`.

Can turn entity copies `STALE`.

```c++
template <typename Component>
(const) Component& get_component() (const) 
```
`Returns`: The `Component` requested.

`Prerequisites`: `entity` is `OK`.

`Throws`: `bad_entity` if the `entity` is not `OK`. `invalid_component` if the `entity` does not own a `Component`.

```c++
template <typename Tag>
bool has_tag() const 
```
`Returns`: `bool` indicating if the `entity` has `Tag`.

`Prerequisites`: `entity` is `OK`.

```c++
template <typename Tag>
bool set_tag(bool set) 
```
`Returns`: `bool` indicating if the `entity` had `Tag` before the call. The old value of `has_tag()`.

`Prerequisites`: `entity` is `OK`.

`Throws`: `bad_entity` if the `entity` is not `OK`.

Can invalidate a `for_each` involving `Tag`, only if `set != set_tag<Tag>(set)`.

Can turn entity copies `STALE`.

```c++
bool sync()
```

`Returns`: `true` if the entity is still alive, false otherwise.

`Prerequisites`: `entity` is not `UNINITIALIZED`.

```c++
void destroy()
```
`Prerequisites`: `entity` is `OK`.

`Throws`: `bad_entity` if the `entity` is not `OK`.

Can invalidate a `for_each`.

Turns entity copies 'DELETED'.

#### `entity` vs `entity_t`
`entity` is the template class while `entity_t` is the template class with the same template arguments as the `entity_manager`. That is, `entity_t = entity<component_list, tag_list>`.

### Entity Manager
```c++
template <typename... Tags, typename... Components>
entity_t create_entity(Components&&... comps)
```
`Returns`: `entity_t` that was created with the given `Tags` and `Components`.

Can invalidate references to all components of types `Components`, as well as a `for_each` involving `Tags` or `Components`.

```c++
template <typename... Ts>
return_container get_entities() 
```
`Returns`: `return_container` of all the entities that have all the components/tags in `Ts...`.

```c++
template <typename... Ts, typename Func>
void for_each(Func && func)
```
Calls `func` for each entity that has all the components/tags in `Ts...`. The arguments supplied to `func` are the entity, as well as all the components in `Ts...`.

```c++
std::size_t get_max_linear_dist() const
```

```c++
void set_max_linear_dist(std::size_t)
```

```c++
void set_event_manager(const event_manager &)
```

```c++
void clear_event_manager()
```

```c++
template <typename... Ts>
entity_grouping create_grouping()
```
`Returns`: `entity_grouping` of the grouping created.


### Entity Grouping
```c++
bool is_valid()
```
`Returns`: `true` if the entity grouping exists, `false` otherwise.

```c++
bool destroy()
```
`Returns`: `true` if the grouping was destroyed, `false` if the grouping doesn't exist.

### Event Manager
```c++
template <typename Event, typename Func>
subscriber_handle<Event> subscribe(Func && func);
```
`Returns`: A `subscriber_handle` for the `func`.

```c++
template <typename Event>
void broadcast(const Event &event) const
```

### Subscriber Handle
```c++
bool isValid() const
```
`Returns`: `true` if the handle holds onto a subscribed function, `false` otherwise.

```c++
bool unsubscribe()
```
`Returns`: `true` if the subscribed function was unsubscribed, `false` if there is no subscribed function.
