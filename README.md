# EntityPlus
EntityPlus is an Entity Component System library written in C++14, offering fast compilation and runtime speeds (to be benchmarked). The only external dependency is `boost` (specifically `boost::container`), and the library is header only, saving you the trouble of fidgeting with build systems.

The ECS framework is an attempt to decouple data from mechanics. In doing so, it lets you create objects out of building blocks that mesh together to create a whole. It models a has-a relationship, letting you expand without worrying about dependency trees and inheritance. The three main aspects of an ECS framework are of course Entities, Components, and Systems.

## Requirements
EntityPlus requires C++14 conformance, and was mainly developed on MSVC. It has been tested to work on

* MSVC 2015 update 3
* Clang 3.5.0
* GCC 5.3.0

The only other requirement is boost.

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
using TagList = tag_list<>;
entity_manager<CompList, TagList> entityManager;
```
Don't be scared by the syntax. Since we don't rely on inheritance or CRTP, we must give the `entity_manager` the list of components we will use with it, as well as a list of [tags](#tags). To create a list of components, we simply use a `component_list`. `component_list`s and `tag_list`s have to be unique, and the `component_list` and `tag_list` can't have overlapping types. If you mess up, you'll be told via compiler error.
```c++
error C2338: component_list must be unique
```
Not so bad, right? EntityPlus is designed with the end user in mind, attempting to achieve as little template error bloat as possible. Almost all template errors will be reported in a simple and concise manner, with minimal errors as the end goal. With `C++17` most code will switch over to using `constexpr if` for errors, which will reduce the error call stack even further.

Now that we have a manager, we can create an actual entity.
```c++
auto entity = entityManager.create_entity();
```
You probably want to add those components to the entity.
```c++
auto retId = entity.add_component<identity>("John", 25);
retId.first.name_ = "Smith";
entity.add_component<health>(100, 100);
```
If we supply a component that wasn't part of the original component list, we will be told this at compile time. In fact, any sort of type mismatch will be presented as a user friendly error when you compile. Adding a component is quite similar to using `map::emplace()`, because the function forwards its args to the constructor and has a similar return semantic. A `pair<component&, bool>` is returned, indicating error or success and the component. The function can fail if a component of that type already exists, in which case the returned `component&` is a reference to the already existing component. Otherwise, the function succeeded and the new component is returned. We don't hold on to the return of `create_entity()` for long. In fact, storing a stale entity is incorrect, as modifying the entity invalidates all references to it. You'll be treated with a runtime error if you use a stale entity.

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
    std::cout << id.name << "\n";
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
Events are orthogonal to ECS, but when used in conjunction they create better decoupled code. You can register an event handler and dispatch events which informs all the handlers.

```c++
event_manager<entity_created, entity_destoryed> eventManager;
eventManager.register_handler<entity_created>([](const auto &) {
    std::cout << "Entity was created\n";
});
eventManager.broadcast(entity_created{});
```

### Exceptions and Error Codes
EntityPlus can be configured to use either exceptions or error codes. The two types of exceptions are `invalid_component` and `bad_entity`, with corresponding error codes. The former is thrown when `get_component()` is called for an entity that does not own a component of that type. The latter is thrown when an entity is stale, belongs to another entity manager, or when the entity has already been deleted. These states can be queried by `get_status()` which returns a corresponding `entity_status`.

To enable error codes, you must `#define ENTITYPLUS_NO_EXCEPTIONS` and `set_error_callback()`, which takes a `std::function<void(error_code_t code, const char *msg)>` as an argument.

## Performance
EntityPlus was designed with performance in mind. Almost all information is stored contiguously (through `flat_map`s and `flat_set`s) and the code has been optimized for iteration (over insertion/deletion) as that is the most common operation when using ECS. 

### Benchmarks
I've benchmarked EntityPlus against EntityX, another ECS library for C++11 on my Lenovo Y-40 which has an i7-4510U @ 2.00 GHz. Compiled using MSVC 2015 update 3 with hotfix on x64. The source for the benchmarks can be viewed [here](entityplus/benchmark.cpp). The time to add the components was very negligible and unlikely to impact performance much in the long run unless you're adding/removing components more than you are iterating over them.

```
Entity Count | Iterations | Probability | EntityPlus | EntityX
----------------------------------------------------------------
    1 000    | 1 000 000  |    1 in 3   |   1484 ms  |  20959 ms
   10 000    | 1 000 000  |    1 in 3   |  15916 ms  | 208156 ms
   30 000    |   100 000  |    1 in 3   |   5092 ms  |  63012 ms
  100 000    |   100 000  |    1 in 5   |  13946 ms  | 133365 ms
   10 000    | 1 000 000  |  1 in 1000  |   5334 ms  |  16883 ms
  100 000    | 1 000 000  |  1 in 1000  |  86312 ms  | 170271 ms
   
```

### Big O Analysis
```c++
n = amount of entities

Entity:
has_(component/entity) = O(1)
(add/remove)_component = O(n)
set_tag = O(n)
get_component = O(log n)
get_status = O(log n)

Entity Manager:
(create/remove)_entity = O(n)
get_entities = O(n)
for_each = O(n)
```

## Reference
### Entity:
```c++
entity_status get_status() const noexcept
```
`Returns`: Status of `entity`, one of `OK`, `INVALID_MANAGER`, `NOT_FOUND`, or `STALE`.


```c++
template <typename Component>
bool has_component() const noexcept
```
`Returns`: `bool` indicating whether the `entity` has the `Component`. 

`Prerequisites`: `entity` is `OK`.


```c++
template <typename Component, typename... Args>
std::pair<Component&, bool> add_component(Args&&... args)
```
`Returns`: `bool` indicating if the `Component` was added. If it was, a reference to the new `Component`. Otherwise, the old `Component`. Does not overwrite old `Component`.

`Prerequisites`: `entity` is `OK`.

`Throws`: `bad_entity` if the `entity` is not `OK`.

Can invalidate references to all components of type `Component`, as well as a `for_each` involving `Component`.


```c++
template <typename Component>
bool remove_component()
```
`Returns`: `bool` indicating if the `Component` was removed.

`Prerequisites`: `entity` is `OK`.

Can invalidate references to all components of type `Component`, as well as a `for_each` involving `Component`.


```c++
template <typename Component>
(const) Component& get_component() (const) 
```
`Returns`: The `Component` requested.

`Prerequisites`: `entity` is `OK`.

`Throws`: `bad_entity` if the `entity` is not `OK`. `invalid_component` if the `entity` does not own a `Component`.


```c++
template <typename Tag>
bool has_tag() const noexcept 
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

Can  invalidate a `for_each` involving `Tag`, only if `set != set_tag<Tag>(set)`.

### Entity Manager:
```c++
entity_t create_entity()
```
`Returns`: `entity_t` that was created.

Can invalidate a `for_each`.


```c++
void delete_entity(const entity_t &entity)
```
`Prerequisites`: `entity` is `OK`.

`Throws`: `bad_entity` if the `entity` is not `OK`.

Can invalidate a `for_each`.


```c++
template<typename... Ts>
return_container get_entities() noexcept
```
`Returns`: `return_container` of all the entities that have all the components/tags in `Ts...`.


```c++
template<typename... Ts, typename Func>
void for_each(Func && func)
```
Calls `func` for each entity that has all the components/tags in `Ts...`. The arguments supplied to `func` are the entity, as well as all the components in `Ts...`.

####`entity` vs `entity_t`
`entity` is the template class while `entity_t` is the template class with the same template arguments as the `entity_manager`. That is, `entity_t = entity<component_list, tag_list>`.
