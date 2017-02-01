# EntityPlus
EntityPlus is an Entity Component System written in C++14, offering fast compilation and runtime speeds (to be benchmarked). The only external dependency is `boost` (specifically `boost::container`), and the library is header only, saving you the trouble of fidgeting with build systems.

An Entity Component System is an attempt to decouple data from mechanics. In doing so, it lets you create objects out of building blocks that mesh together to create a whole. It models a has-a relationship, letting you expand without worrying about dependency trees and inheritence. The three main aspects of an ECS are of course Entities, Components, and Systems.

###Components
Components contain information. This can be anything, such as health, a peice of armor, or a status effect. An example component could be the identity of a person, which could be modeled like this:
```c++
struct identity {
    std::string name_;
    int age_;
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
As you may have noticed, these are just free `class`es. Usually, to use them with other ECSs you'd have to make them inherit from some common base, probably along with CRTP. However, EntityPlus takes advantage of the type system to eliminate these needs. To use these `class`es we have to simply specify them later.

###Entities
Entities model something. You can think of them as containers for components. If you want to model a player character, you might want a name, a measurement of their health, and an inventory. To use an entity, we must first create it. However, you can't just create an `entity`, it needs context on where it exists. We use an `entity_manager` to manage all our `entity`s for us.
```c++
using CompList = component_list<identity, health>;
using TagList = tag_list<>;
entity_manager<CompList, TagList> entityManager;
```
Don't be scared by the syntax. Since we don't rely on inheritence or CRTP, we must give the `entity_manager` the a list of components we will use with it, as well as a list of tags. Don't worry about tags, we'll cover those later. To create a list of components, we simply use a `component_list`. `component_list`s and `tag_list`s have to be unique, and the `component_list` and `tag_list` can't have overlapping types. You'll be told about this via compiler error.
```c++
error C2338: ComponentList must be unique
```
Not so bad right? EntityPlus is designed with the end user in mind, attempting to achieve as little template errors as possible. Almost all template errors will be reported in a simple and concise manner, with minimal errors as the end goal. With `c++17` most code will switch over to using `constexpr if` for errors, which will reduce the error callstack even further.

Now that we have a manager, we can create an actual entity.
```c++
auto entity = entity_manager.create_entity();
```
You probably want to add those components to the entity.
```c++
auto retId = entity.addComponent<identity>("John", 25);
retId.first.name_ = "Smith";
entity.addComponent<health>(100, 100);
```
It's quite similair to using `vector::emplace`, because the function forwards it's args to the constructor. What gets returned is a `pair<component&, bool>`, which contains the component you just added, and a `bool` indicating if you overwrote the previous component. We don't hold on to the return of `create_entity()` for long. In fact, storing a stale entity is incorrect, as modifying the entity invalidates all references to it. You'll be treated with a runtime error if you use a stale entity.

###Systems
The last thing we want to do is manipulate our entities. Unlike some ECS frameworks, EntityPlus doesn't have a system manager or similar device. You can work with the entities in one of two ways. The first is querying for a list of them by type
```c++
auto ents = entityManager.get_entities<identity>();
for (const auto &ent : ents) {
    std::cout << ent.get_component<identity>().name_ << "\n";
}
```
`get_entities()` will return a `vector` of all the entities that contain the given components and tags.

The second, and faster way, of manipulating entities is by using lambdas (or any `callable` really).
```c++
entityManager.for_each<identity>([](auto ent, auto &id) {
    std::cout << id.name << "\n";
}
```
That's about it! You can obviously wrap these methods in your own system classes, but having specific support for systems felt artifical and didn't add any impactful or useful changes to the flow of usage.

###Tags
Tags are like components that have no data. They are simply a typename(and don't even have to be complete types) that is attached to an entity. An example could be a player tag for the entity that is controlled by a player. Tags can be used in any way a component is, but since there is no value associated with it except if it exists or not, it can only be toggled.

```c++
ent.set_tag<player_tag>(true);
assert(ent.get_tag<player_tag>() == true);
```

###Events
Events are diagonal to ECS, but when used in conjunction they create better decoupled code. You can register an event handler and also dispatch an event which informs all the handlers.

###Exceptions and Error Codes
EntityPlus can be configured to use either exceptions or error codes.

###Performance
EntityPlus was designed with performance in mind. Almost all information is stored contigously (through `flat_map`s and `flat_set`s) and the code has been optimized for iteration(over insertion/deletion) as that is the most common operation when using ECS. Here are the big O analysis of the functions.

```c++
n = amount of entities

Entity:
has_(component/entity) = O(1)
add_component = O(n)
remove_component = O(n)
get_component = O(log n)
set_tag = O(n)

Entity Manager:
create_entity = O(n)
remove_entity = O(n)
get_entities = O(n^2) // should be optimized
for_each = O(n)
```
